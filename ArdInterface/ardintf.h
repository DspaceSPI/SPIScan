/*
 *  ardintf.h
 *  ArdInterface
 *
 *  Created by michael on 13/05/12.
 *  Copyright 2012 Divaricating Systems. All rights reserved.
 *
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "SPIproto.h"

typedef unsigned char uint8_t;

void ard_init(int serFile);
void ard_startup(void);

// SPIscan known pins
enum {
  kOnboardLEDPin = 13,
  kPump1Pin = 16,
  kPump2Pin = 17
  
};

// ----------------- Arduino API -----------------------

#define HIGH 0x1
#define LOW  0x0
#define INPUT 0x0
#define OUTPUT 0x1

void pinMode(uint8_t pin, uint8_t mode);
void digitalWrite(uint8_t pin, uint8_t value);
int  digitalRead(uint8_t pin);
int  analogRead(uint8_t pin);
void analogWrite(uint8_t pin, int value);

// ----------------- Native low-level messaging API ----------------------

void SendMsg(Sp_Command cmd, uint8_t transID, const void* p, uint8_t len);
bool HasMessageFor(uint8_t targID);
void ReadMessageFor(uint8_t ID, Sp_Command* outCmd, void* outBuffP, uint8_t* ioBuffSize);



#ifdef __cplusplus
}
#endif
