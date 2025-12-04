#ifndef SIM_H
#define SIM_H

#include <Arduino.h>
#include <MK_SIM_A7600C.h>

#define SIM_SERIAL_RX_PIN 27
#define SIM_SERIAL_TX_PIN 13

#define SIM_SERIAL Serial1
#define SIM_BAUDRATE (115200)
#define SIM_SERIAL_CONFIG SERIAL_8N1

void SIM_Init();
void SIM_task(void *pvParameters);
void CheckBalance();
String decodeUCS2(const String &hex);


#endif

