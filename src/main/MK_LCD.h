#pragma once
#include <LiquidCrystal_I2C.h>

#ifdef KEYPAD_DEBUG
#include "MK_Keypad.h"
#endif

#define LCD_ROWS 16
#define LCD_COLS 2
#define LCD_ADDRESS 0x27

extern LiquidCrystal_I2C lcd;

void Module_LCD_init();
void taskMonitor_LCD(void* pvParameters);
