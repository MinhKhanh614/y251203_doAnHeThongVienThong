// Add the library
#include <MKL_LiquidCrystal_I2C.h>

// LCD config
MKL_LiquidCrystal_I2C lcd(0x27, 20, 4);

void setup() {
  // Initialize LCD
  lcd.init();
  lcd.backlight();

  // Display a welcome message
  lcd.setCursor(0, 0);
  lcd.print("Welcome to");
  lcd.setCursor(0, 1);
  lcd.print("LCD2004 Demo");
}

void loop() {
  // No additional functionality for now
}