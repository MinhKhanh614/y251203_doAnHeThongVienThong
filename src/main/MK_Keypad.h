#pragma once
#include <Keypad.h>
#include <Arduino.h>

#define PIN_COL_1 25
#define PIN_COL_2 26
#define PIN_COL_3 32
#define PIN_COL_4 33

#define PIN_ROW_1 16
#define PIN_ROW_2 17
#define PIN_ROW_3 18
#define PIN_ROW_4 19

#define KEYPAD_COLS 4
#define KEYPAD_ROWS 4

void keypad_init();
void taskREAD_KEYPAD(void* pvParameters);

extern volatile char key;