#include "MK_Keypad.h"

volatile char key = '0';

// Keypad 4x4
const byte ROWS = KEYPAD_ROWS;
const byte COLS = KEYPAD_COLS;

char hexaKeys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

byte rowPins[ROWS] = {PIN_ROW_1,PIN_ROW_2,PIN_ROW_3,PIN_ROW_4};
byte colPins[COLS] = {PIN_COL_1,PIN_COL_2,PIN_COL_3,PIN_COL_4};

Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);

void keypad_init()
{
  xTaskCreate(
    taskREAD_KEYPAD
    ,  "READ_KEYPAD"   // A name just for humans
    ,  4096             // This stack size can be checked & adjusted by reading the Stack Highwater
    ,  NULL
    ,  1
    ,  NULL );
}

void taskREAD_KEYPAD(void* pvParameters)
{
  while(1)
  {
    char k = customKeypad.getKey();
    if(customKeypad.getState() == PRESSED)
    {
      if (k != NO_KEY) key = k; // tránh lặp phím
    }
    vTaskDelay(pdMS_TO_TICKS(50)); // delay 50ms để tránh lặp phím
  }
}
