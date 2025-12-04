// Add the library
#include <MKL_LiquidCrystal_I2C.h>

// LCD config
MKL_LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
  // Initialize LCD
  lcd.init();
  lcd.backlight();

  // Display a welcome message
  lcd.setCursor(0, 0);
  lcd.print("Welcome to");
  lcd.setCursor(0, 1);
  lcd.print("LCD1602 Demo");
}

void loop() {
  // No additional functionality for now
  int numb = 0;
  for ( uint8_t i = 0; i < 2; i++)
  {
    for ( uint8_t j = 0; j < 16; j++)
    {
      lcd.setCursor(j, i);
      lcd.print(numb);
      numb++;
      delay(500);
    }
  }
  lcd.clear();
  delay(1000);
}