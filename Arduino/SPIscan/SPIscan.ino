/*
Process serial commands from the host and send responses
Serial packet protocol is:
 <sync>       Byte, always 0xAA
 <command>    Byte
 <transID>    Byte
 <paramLen>   Byte
 <parameters> Byte array, paramLen bytes (could be 0)
 <checksum>   Byte, low byte of sum of <sync>..<last parameter>
 
<transID> is sent by the host and used in any response(s) and can be any value
except 00, which is reserved for messages sent by the Arduino without being asked.

The Arduino is predefined as driving:
 - two pump relays on RXD2 and TXD2
 - a PWM LED drive PH4
 - a laser enable PA0
 - a temp sensor on PC7 (A15)
 
*/

#include "SPIproto.h"      // Serial protocol constants

// Configurable parameters
enum {
  kBaudRate = 115200,
  kMaxMsgSize = 50,      // Max data size
  
  kTotalSize = kHdrSize + kMaxMsgSize + kCksumSize,
};

// Less configurable parameters
const char kSysInfoStr[] = "SPIScan " __DATE__ __TIME__ ;

//====================== Predefined pins  ======================

enum {
  kPredefIn,
  kPredefDigOut,
  kPredefAnaOut
};

struct KnownPin {
  byte pinNum;
  byte outDir;
  byte outState;
};

//------------------- Configurable ----------------

enum {
  kOnboardLEDPin = 13,
  kPump1Pin = 16,
  kPump2Pin = 17
  
};

const KnownPin gPredefinedPins[] = {
  { kOnboardLEDPin, kPredefDigOut, LOW },    // Onboard LED
  { kPump1Pin, kPredefDigOut, LOW },    // PH0(RXD2) pump drive
  { kPump2Pin, kPredefDigOut, LOW },    // PH1(TXD2) pump drive
  { 7,  kPredefAnaOut, 128 },    // PH4 LED drive (PWM)
  { 22, kPredefDigOut, LOW }     // PA0 laser enable
};

enum { kNPredefs = sizeof(gPredefinedPins)/sizeof(KnownPin) };

//======================= Globals ========================
byte   gInMsg[kTotalSize];
byte   gCkSum = 0;
Sp_Error   gNetErr = kNoErr;      // Last error 
long   gNetParam;    // Last error parameter
short  gInWrIdx = 0;

// ----------------------- Packet protocol --------------------------

void SendMsg(Sp_Command cmd, byte transID, const void* p, byte len)
{
  int i;
  byte header[kHdrSize] = { kSyncByte, cmd, transID, len };
  byte cksum = kSyncByte + cmd + transID + len;
  
  Serial.write(&header[0],kHdrSize);
  if (len>0) {
     const byte* bp = (byte*)p;
     for (i=0; i<len; i++)
        cksum += bp[i];
     Serial.write(bp,len);
  }
  Serial.write(cksum);
}

void SendErrMsg(Sp_Error errcode, byte transID, long parameter)
{
  uint8_t msg[5] = { errcode, parameter>>24, parameter>>16, parameter>>8, parameter & 0xFF };
  SendMsg(kCCError, transID, &msg, sizeof(msg));
}

void SendMsgOrNetErr(Sp_Command resp, byte transID, void* dataP, byte nBytes)
// Either send specified message or an error code if gNetErr is nonzero: useful
// for responding to list messages
{
  if (gNetErr==kNoErr)
    SendMsg(resp,transID,dataP,nBytes);
  else
    SendErrMsg(gNetErr,transID,gNetParam);
}

bool GetMsg()
// Call whenever convenient, will return true if a valid and complete message is in the gInMsg buffer
// If it returns true you *must* process the message before calling again
{
   int i,n;
   byte b;
   
   n = Serial.available();
   
   for (i=0; i<n; i++) {
     b = Serial.read();
     //Serial.write(b);
     if (gInWrIdx==0) {
       if (b==kSyncByte) {
         gInMsg[gInWrIdx++] = b;
         gCkSum = b;
       }
     } else {
        if (gInWrIdx < kTotalSize)
          gInMsg[gInWrIdx++] = b;
        if (gInWrIdx > kLenPos && gInWrIdx >= kHdrSize + gInMsg[kLenPos] + kCksumSize) {
            // Message complete; reset wridx
            gInWrIdx = 0;
            if (gCkSum != b) {
               return false;      // Ignore if it has a bad checksum
            } else if (gInMsg[kLenPos]>kMaxMsgSize) {
                SendErrMsg(kErrMsgOversize,gInMsg[kIDPos],gInMsg[kLenPos]);    // valid, but too big for buffer
                return false;
            } else
                return true; 
        } else
           gCkSum += b;
     }
   } // for
   return false;
}

//------------------------ Utilities ---------------------------------------

bool isInput(byte pinNum)
{
  volatile byte* rp = portModeRegister(digitalPinToPort(pinNum));
  return (*rp & digitalPinToBitMask(pinNum)) == 0;
}

bool isPredefined(byte aPin)
{
  int i;
  for (i=0 ; i<kNPredefs; i++) 
     if (gPredefinedPins[i].pinNum==aPin)
       return true;
  return false;
}

//------------------------ Core commands ----------------------------------

void SetMode(byte pinNum, byte value)
{
   // Check its a valid pin number and it doesn't have a predefined direction
   if (digitalPinToPort(pinNum)==NOT_A_PIN) {
      gNetErr = kErrInvalidPin;
      gNetParam = pinNum;
   } else if (isPredefined(pinNum)) {
      gNetErr = kErrPredefined;
      gNetParam = pinNum;
   } else
      pinMode(pinNum,value);
}

void SetPin(byte pinNum, byte value)
{
   // Check its a valid pin number and it isn't an input
   if (digitalPinToPort(pinNum)==NOT_A_PIN) {
      gNetErr = kErrInvalidPin;
      gNetParam = pinNum;
   } else if (isInput(pinNum)) {
      gNetErr = kErrOutToInput;
      gNetParam = pinNum;
   } else
      digitalWrite(pinNum,value);
}

byte GetPin(byte pinNum)
{
  // Check its a valid pin number. Its OK to read back from outputs
  if (digitalPinToPort(pinNum)==NOT_A_PIN) {
      gNetErr = kErrInvalidPin;
      gNetParam = pinNum;
      return LOW;
  } else 
      return digitalRead(pinNum);
}

void AnalogReadPinList(byte transID, byte* pinNums, byte nPins)
{
  int i;
  int gValues[kMaxMsgSize/sizeof(int)];
  
  for (i=0; i<nPins; i++)
    // Check for valid analog pin
    if (pinNums[i] > NUM_ANALOG_INPUTS) {
      gNetErr = kErrInvalidPin;
      gNetParam = pinNums[i];
    } else { 
      gValues[i] = analogRead(pinNums[i]);
    }
      
  SendMsgOrNetErr(kCCPinValues, transID, gValues, nPins);
}

void GetPinList(byte transID, byte* pinNums, byte nPins)
{
  int i;
  byte gValues[kMaxMsgSize];
  
  for (i=0; i<nPins; i++)
    gValues[i] = GetPin(pinNums[i]);
  SendMsgOrNetErr(kCCPinStates, transID, gValues, nPins);
}

void SetPinList(byte transID, byte* pinNums, byte nPins)
{
  int i;
  
  for (i=0; i<nPins; i++)
    SetPin(pinNums[i] & 0x7F, (pinNums[i] & 0x80) ? HIGH : LOW);
  SendMsgOrNetErr(kCCNoErr, transID, NULL, 0);
}

void SetModeList(byte transID, byte* pinNums, byte nPins)
{
  int i;
  
  for (i=0; i<nPins; i++)
    SetMode(pinNums[i] & 0x7F, (pinNums[i] & 0x80) ? OUTPUT : INPUT);
  SendMsgOrNetErr(kCCNoErr, transID, NULL, 0);
}

void AnalogWriteList(byte transID, byte* pvList, byte nValues)
{
  int i;
  byte pinNum;
  
  for (i=0; i<nValues; i++) {
    pinNum = pvList[0];
    if (isInput(pinNum)) {
      gNetErr = kErrOutToInput;
      gNetParam = pinNum;
    } else 
      analogWrite(pinNum, pvList[1]);
    pvList += 2;
  }
  SendMsgOrNetErr(kCCNoErr, transID, NULL, 0);
}

void ProcessMsg()
{
  gNetErr = kNoErr;
  switch (gInMsg[kCmdPos]) {
    
    case kCCGetPins:
      GetPinList(gInMsg[kIDPos], &gInMsg[kDataPos], gInMsg[kLenPos]);
      break;
      
    case kCCSetPins:
      SetPinList(gInMsg[kIDPos], &gInMsg[kDataPos], gInMsg[kLenPos]);
      break;
      
    case kCCSetPinModes:
      SetModeList(gInMsg[kIDPos], &gInMsg[kDataPos], gInMsg[kLenPos]);
      break;
    
    case kCCAnalogReads:
      AnalogReadPinList(gInMsg[kIDPos], &gInMsg[kDataPos], gInMsg[kLenPos]);
      break;
    
    case kCCSetPWM:
      if (gInMsg[kLenPos] & 1)    // Must be a list of pin/value pairs
        SendErrMsg(kErrOddPWM,gInMsg[kIDPos],gInMsg[kLenPos]);
      else
        AnalogWriteList(gInMsg[kIDPos], &gInMsg[kDataPos], gInMsg[kLenPos]/2);
      break;
    
    case kCCGetSysInfo: {
         SendMsg(kCCSysInfo,gInMsg[kIDPos],kSysInfoStr,strlen(kSysInfoStr));
      } break;
    
    default:
      SendErrMsg(kErrUnknownCmd, gInMsg[kIDPos], gInMsg[kCmdPos]);
      break;
  }
}


//---------------------------- Arduino main ------------------------

void setup()
{
   int i;
   
   // Setup predefined state
   for (i=0 ; i<kNPredefs; i++) {
     const KnownPin* pinP = &gPredefinedPins[i];
     if (pinP->outDir == kPredefIn)
       pinMode(pinP->pinNum, INPUT);
     else {
       pinMode(pinP->pinNum, OUTPUT);
       if (pinP->outDir == kPredefDigOut) digitalWrite(pinP->pinNum, pinP->outState);
                                     else analogWrite(pinP->pinNum, pinP->outState);
     }
   }
   // Turn on serial port
   gInWrIdx = 0;
   Serial.begin(kBaudRate);
  
   SendMsg(kCCSysInfo,0,kSysInfoStr,strlen(kSysInfoStr));
}

long count = 0;

void loop()
{
   while (GetMsg())
     ProcessMsg();
}


