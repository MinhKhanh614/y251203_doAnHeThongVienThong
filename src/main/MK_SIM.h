#pragma once
#include <Arduino.h>



#define SIM_SERIAL_RX_PIN 13
#define SIM_SERIAL_TX_PIN 27

#define SIM_SERIAL Serial1
#define SIM_BAUDRATE 115200
#define SIM_SERIAL_CONFIG SERIAL_8N1

void Module_SIM_Init();
String readIncomingNumber();
void IRAM_ATTR simCallback();

extern String incomingNumber;
extern bool flagSIM;