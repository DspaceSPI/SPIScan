/*
 *  ardintf.c
 *  ArdInterface
 *
 *  Created by michael on 13/05/12.
 *  Copyright 2012 Divaricating Systems. All rights reserved.
 *
 */


#include <stdio.h>
#include <stdbool.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <termios.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
static void declare_ard_error(void);

#include "ardintf.h"

static pthread_mutex_t ard_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t ard_cond = PTHREAD_COND_INITIALIZER;
static unsigned char ard_running=0, ard_error=0;

#define qLogUART
#define qLogMsgs

enum { 
    kRdBuffSize = 1024,
    kMaxInBytes = 128 
};

typedef struct InMsg {
    Sp_Command cmd;
    uint8_t    id;
    uint8_t    dataLen;
    uint8_t    data[kMaxInBytes];
} InMsg;
    
static int      gFile = 0;
static uint8_t  gInBuffer[kRdBuffSize];
static int      grdex, gwdex;
static uint8_t  gID;


void errstr(const char* str, int value)
{
    printf("Error : %s : %d\n",str,value);
}

//-------------------------- Packet primitives -----------------------

void ard_init(int serFile)
{
    struct termios options;
    uint8_t syncBuffer[56];
    int err;

    gFile = serFile;
    grdex = 0;
    gwdex = 0;
    gID = 1;
    
    // Set up serial port with POSIX commands
    tcgetattr(serFile,&options);
    cfmakeraw(&options);
    cfsetspeed(&options,115200);  // Baud rate
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;         // 8 bits
    options.c_cflag &= ~PARENB;     // No parity
    options.c_cflag &= ~CSTOPB;     // One stop bit
    options.c_cc[VMIN] = 0;         // Minimum number of chars to read
    options.c_cc[VTIME] = 1;        // Timeout in 1/10 second units

    if ( tcsetattr(serFile,TCSANOW,&options) < 0 )
        errstr("Serial setup failed",errno);
                
    // Synchronise with Arduino in unknown state by sending a burst of 01 01 01...
    // This should help ensure it is looking for sync
    memset(syncBuffer,1,sizeof(syncBuffer));
    err = write(serFile,syncBuffer,sizeof(syncBuffer));
    if (err < 0 && errno != EAGAIN)
	  declare_ard_error();
    #ifdef qLogUART
        printf("Tx: resync burst\n");
    #endif
    
    // Send a dummy command that will be ignored, this helps if we lose the first
    // few characters of input at the host end
    //SendMsg(kCCInvalid, 1, NULL, 0);
}

void LoadBuffer()
// Call anytime, buffers serial port reads into gInBuffer
{
    int i, nmax, nbytes;
    
    if (grdex > gwdex) nmax = (grdex - gwdex) - 1;
                  else nmax = kRdBuffSize - gwdex;
    nbytes = read(gFile, &gInBuffer[gwdex], nmax);
    if (nbytes > 0) {
        #ifdef qLogUART
            printf("Rx: ");
            for (i=gwdex; i<gwdex+nbytes; i++)
                printf("%02X ",gInBuffer[i]);
            printf("\n");
        #endif
        gwdex += nbytes;
        if (gwdex >= kRdBuffSize) gwdex = 0;
    } else if (nbytes < 0) {
        if (errno != EAGAIN) {    // EAGAIN is normal, means no data
	    declare_ard_error();
            errstr("read failed",errno);
	}
    }
}

int InBuffSize()
{
    if (gwdex >= grdex)
        return gwdex - grdex;
    else
        return kRdBuffSize - (grdex-gwdex);
}

void SendMsg(Sp_Command cmd, uint8_t transID, const void* p, uint8_t len)
// Call anytime, sends
{
  int i,err;
  uint8_t header[kHdrSize] = { kSyncByte, cmd, transID, len };
  uint8_t cksum = kSyncByte + cmd + transID + len;
  const uint8_t* bp = (const uint8_t*)p;
     
  write(gFile, &header, kHdrSize);
  if (len>0) {
     for (i=0; i<len; i++)
        cksum += bp[i];
     write(gFile,bp,len);
  }
  err = write(gFile,&cksum,1);
  if (err<0) {
    if (errno != EAGAIN)
	  declare_ard_error();
    printf("write failed %d\n",errno);
  }
  
  #ifdef qLogUART
    printf("Tx: ");
    for (i=0;i<sizeof(header);i++)
        printf("%02X ",header[i]);
    printf("[");
    for (i=0; i<len; i++)
        printf("%02X ",bp[i]);
    printf("]%02X\n",cksum);
  #endif
  LoadBuffer();
}

uint8_t NextByte()
// Read a byte from gInBuffer
{
    uint8_t result = gInBuffer[grdex];
    if (++grdex>=kRdBuffSize) grdex = 0;
    return result;
}


bool GetMsg(InMsg* msg)
// Parse a message from gInBuffer, if one is available. 
// Discards messages with bad checksums, oversize messages (kMaxInBytes < 255)
{
    int sdex, i, nskip;
    uint8_t cksum;
    
    // Do a serial read
    LoadBuffer();
    
    // Scan forward until we start at a sync byte
    nskip = 0;
    while (grdex!=gwdex && gInBuffer[grdex]!=kSyncByte) {
        if (++grdex>=kRdBuffSize) grdex = 0;
        nskip++;
    }
    if (nskip>0)
        errstr("Skipped bytes searching for sync",nskip);
        
    // Do we have at least a header plus checksum (smallest possible message)?
    if (InBuffSize() < kHdrSize+1)
        return false;
    else {
        // Look ahead in the buffer for the length byte
        sdex = grdex+3;
        if (sdex > kRdBuffSize) sdex -= kRdBuffSize;
        // Do we have a header plus the necessary length plus a checksum byte?
        if (InBuffSize() >= kHdrSize + gInBuffer[sdex] + 1) {
            //Yes, return the message
            uint8_t sum = NextByte();  
            
            msg->cmd = (Sp_Command)NextByte();
            sum += msg->cmd;
            msg->id = NextByte();
            sum += msg->id;
            msg->dataLen = NextByte();
            sum += msg->dataLen;
        
            for (i=0 ; i<msg->dataLen; i++) {
                uint8_t b = NextByte();
                sum += b;
                if (i < kMaxInBytes) 
                    msg->data[i] = b;
            }
            cksum = NextByte();
            if (cksum != sum)
                errstr("checksum failure",sum);
            else if (msg->dataLen > kMaxInBytes)
                errstr("message oversize",msg->dataLen);
            else {
                #ifdef qLogMsgs
                    printf("Got message type $%X, ID$%X, %d bytes\n",msg->cmd, msg->id, msg->dataLen);
                #endif
            }
            return cksum == sum && msg->dataLen <= kMaxInBytes;
        }
    }
    return false;
}

uint8_t NextID()
{
    if (++gID==0) gID = 1;
    return gID;
}

//--------------------- Packet queue ----------------------------

typedef struct MsgQItem {
    InMsg msg;
    struct MsgQItem* next;
    struct MsgQItem* prev;
} MsgQItem;

MsgQItem* gQFront = NULL;
MsgQItem* gQBack = NULL;


// Keep a single free buffer to reduce malloc/free thrashing
MsgQItem* gTemp = NULL;

MsgQItem* NewItem()
{
    if (gTemp==NULL)
        return (MsgQItem*)malloc(sizeof(MsgQItem));
    else {
        MsgQItem* result = gTemp;
        gTemp = NULL;
        return result;
    }
}

void FreeItem(MsgQItem* p)
{
    if (gTemp==NULL) gTemp = p;
                else free(p);
}

// Double-linked list, push to back, ReadMsgQ by key search starting at front
void Push(MsgQItem* item)
{
    item->prev = NULL;
    if (gQFront==NULL) {
        gQFront = item;
        gQBack = item;
        item->next = NULL;
	pthread_cond_signal(&ard_cond);
    } else {
        item->next = gQBack;
        gQBack->prev = item;
        gQBack = item;
    }
}

void ReadToQ()
{
    MsgQItem* temp = NewItem();
    while ( GetMsg(&temp->msg) ) {
        Push(temp);
        temp = NewItem();
    }
    FreeItem(temp);
}

MsgQItem* ReadMsgQ(uint8_t targID)
{
    MsgQItem* scan;
    
    ReadToQ();
    scan = gQFront;
    while (scan != NULL) {
        if (scan->msg.id == targID) {
            if (scan->next == NULL)
                gQFront = scan->prev;
            else 
                scan->next->prev = scan->prev;
            if (scan->prev == NULL)
                gQBack = scan->next;
            else 
                scan->prev->next = scan->next;
            scan->next = NULL;
            scan->prev = NULL;
            break;
        }
        scan = scan->prev;
    }
    return scan;
}


//------------------------ Reply API ---------------------------


void Waiting()
// Gets called repeatedly while API is waiting for a synchronous reply
{
    while (ard_running && !ard_error && !gQFront)
		pthread_cond_wait(&ard_cond, &ard_mutex);
}

void GetAck(uint8_t id)
// Synchronous: wait for a kCCNoErr or kCCError response to ID
{
    MsgQItem* answer = 0;
    
    pthread_mutex_lock(&ard_mutex);
    while (ard_running && (answer = ReadMsgQ(id)) == NULL )
        Waiting();
    if (answer) {
    	if (answer->msg.cmd != kCCNoErr) {
        	if (answer->msg.cmd == kCCError)
            		errstr("Command failed ",answer->msg.data[0]);
        	else 
            		errstr("Unexpected response type",answer->msg.cmd);
    	}
    	FreeItem(answer);
    }
    pthread_mutex_unlock(&ard_mutex);
}

void GetResponse(uint8_t id, InMsg* msgP)
// Synchronous, will wait indefinitely
{
     MsgQItem* answer;
    
    pthread_mutex_lock(&ard_mutex);
    while (ard_running && (answer = ReadMsgQ(id)) == NULL )
        Waiting();
    if (answer) {
    	*msgP = answer->msg;
    	FreeItem(answer);
    }
    pthread_mutex_unlock(&ard_mutex);
}


//---------------- Arduino-style API, synchronous, no error handling ---------------------

void pinMode(uint8_t pin , uint8_t mode)
{
    uint8_t id = NextID();
    uint8_t param = pin | (mode ? 0x80 : 0);
    
    SendMsg(kCCSetPinModes, id, &param, 1);
    GetAck(id);
}

void digitalWrite(uint8_t pin, uint8_t value)
{
    uint8_t id = NextID();
    uint8_t param = pin | (value ? 0x80 : 0);
    
    SendMsg(kCCSetPins, id, &param, 1);
    GetAck(id);
}

int  digitalRead(uint8_t pin)
{
    InMsg msg;
    uint8_t id = NextID();
    
    SendMsg(kCCGetPins, id, &pin, 1);
    GetResponse(id,&msg);
    if (msg.cmd==kCCPinStates )
        return msg.data[0];
    else 
        return 0;
}

int  analogRead(uint8_t pin)
{
    InMsg msg;
    uint8_t id = NextID();
    
    SendMsg(kCCGetPins, id, &pin, 1);
    GetResponse(id,&msg);
    if (msg.cmd==kCCPinValues )
        return msg.data[0]<<8 | msg.data[1];
    else 
        return -1;

}

void analogWrite(uint8_t pin, int value)
{
    uint8_t id = NextID();
    uint8_t params[2] = { pin, value };
    SendMsg(kCCSetPWM, id, &params, sizeof(params));
    GetAck(id);
}

// ---------------- Better API, async -------------------


bool HasMessageFor(uint8_t targID)
{
    MsgQItem* scan;
    
    pthread_mutex_lock(&ard_mutex);
    ReadToQ();
    scan = gQFront;
    while (scan != NULL) {
        if (scan->msg.id == targID) {
    		pthread_mutex_unlock(&ard_mutex);
                return true;
	}
        scan = scan->prev;
    }
    pthread_mutex_unlock(&ard_mutex);
    return false;
}

void ReadMessageFor(uint8_t ID, Sp_Command* outCmd, void* outBuffP, uint8_t* ioBuffSize)
{
    MsgQItem* answer;
    
    pthread_mutex_lock(&ard_mutex);
    while ( (answer = ReadMsgQ(ID)) == NULL )
        Waiting();
    *outCmd = answer->msg.cmd;
    *ioBuffSize = answer->msg.dataLen;
    memcpy(outBuffP,&answer->msg.data,answer->msg.dataLen);
    FreeItem(answer);
    pthread_mutex_unlock(&ard_mutex);
}

static void
declare_ard_error()
{
	pthread_mutex_lock(&ard_mutex);
	ard_error = 1;
	ard_running = 0;
	pthread_cond_broadcast(&ard_cond);
	pthread_mutex_unlock(&ard_mutex);
}

// ---------------------- added by Paul - should fork off a thread to:
//
//	(we need to be able to handle the USB being unplugged and plugged in again
//
//	- open /dev/ttysomething
//	- if it fails delay, retry until 
//	- call ard_init
//	- monitor state, if the tty gets an error (usb is unplugged) 
//

#define ARDUINO_TTY	"/dev/ttyUSB0"

void *
ard_thread(void*pp)
{
	pthread_detach(pthread_self());
	ard_running = 0;
	for (;;) {
		int fd;

		pthread_mutex_lock(&ard_mutex);
		fd = open(ARDUINO_TTY, O_RDWR);
		if (fd < 0) {
			pthread_mutex_unlock(&ard_mutex);
			sleep(1);
			continue;
		}
		ard_error = 0;
		ard_init(fd);
		ard_running = 1;
		while (ard_running && !ard_error)
			pthread_cond_wait(&ard_cond, &ard_mutex);
		ard_running = 0;
		pthread_mutex_unlock(&ard_mutex);
		close(fd);
	}
	return 0;
}

void
ard_startup()	// called from the generic dspace library startup
{
	pthread_t tid;
	pthread_create(&tid, 0, ard_thread, 0); 
}
