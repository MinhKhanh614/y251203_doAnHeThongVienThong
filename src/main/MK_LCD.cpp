#include "MK_LCD.h"
// LCD I2C
LiquidCrystal_I2C lcd(LCD_ADDRESS, LCD_ROWS, LCD_COLS);

void Module_LCD_init()
{
    lcd.init();
    lcd.backlight();

#ifdef KEYPAD_DEBUG
    xTaskCreate(
        taskMonitor_LCD
        , "LCD_TASK" // A name just for humans
        ,  4096 // This stack size can be checked & adjusted by reading the Stack Highwater
        ,  NULL
        , 1
        , NULL);
#endif

}

#ifdef KEYPAD_DEBUG
uint8_t i = 0, j = 0;
void taskMonitor_LCD(void *pvParameters)
{
    while (1)
    {
        lcd.setCursor(i, j);
      
          if(i >= 16){
            i = 0;
            j++;
          }
          
          if(j >= 2) j = 0;
        
          if(key) 
          {
            if (key != '\0' && key != '\r' && key != '\n')
            {

                lcd.print(key);

                Serial.print("Key: ");
                Serial.print(key); // chỉ in ký tự, không thêm \r\n
                Serial.println();  // xuống dòng riêng nếu cần
            }

            key = 0;
            
            i++;
          }
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}
#endif
