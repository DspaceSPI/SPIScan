
/*

Serial packet protocol is:
 <sync>       Byte, always kSyncByte==0xAA
 <command>    Byte
 <transID>    Byte
 <paramLen>   Byte
 <parameters> Byte array, paramLen bytes (could be 0)
 <checksum>   Byte, low byte of sum of <sync>..<last parameter>
 
<transID> is sent by the host and used in any response(s) and can be any value
except 00, which is reserved for messages sent by the Arduino without being asked.

*/


enum {
  kHdrSize = 4,
  kCmdPos = 1,
  kIDPos  = 2,
  kLenPos = 3,
  kDataPos = 4,
  kCksumSize = 1,
  kSyncByte  = 0xAA
};

typedef enum Sp_Command {
  // Responses sent by the SPI
  kCCNoErr,        // No parameters
  kCCError,        // 1 byte error code, 4 bytes error info
  kCCSysInfo,      // Character string
  kCCPinStates,    // pin states, list of HIGH/LOW
  kCCPinValues,    // pin analog values, list of 16 bits MSB-first.
  
  // Command codes sent to the SPI
  kCCGetSysInfo  = 0x80,   // No parameters
  kCCGetPins,           // n byte pin#
  kCCSetPins,           // n byte pin# + 0x80=on  
  kCCSetPinModes,       // n byte pin# + 0x80=output
  kCCAnalogReads,       // n byte pin#
  kCCSetPWM,            // 1 byte pin 1 byte PWM value
  
  kCCInvalid = 0xFE
} Sp_Command;

typedef enum Sp_Error {
  // Error numbers
  kNoErr = 0,
  kErrUnknownCmd,
  kErrMsgOversize,
  kErrInvalidPin,
  kErrOutToInput,
  kErrPredefined,
  kErrOddPWM
} Sp_Error;
