#ifndef PC_CONTROL_H
#define PC_CONTROL_H

#include <Arduino.h>

#define PC_SERIAL Serial
#define PC_BAUDRATE (115200)

void PC_Control_Init();
void taskPC(void *pvParameters);
void taskManager(void *pvParameters);
void mkSerial(Stream *in, Stream *out);


#endif