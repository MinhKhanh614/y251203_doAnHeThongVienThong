// Add the library
#include <MKL_LiquidCrystal_I2C.h>

// LCD config
MKL_LiquidCrystal_I2C lcd(0x27, 16, 2);

// Define the pin for the potentiometer
#define POT_PIN 34 // IO34-A1 on Vietduino ESP32 (A1 on MakerEDU Shield)

void setup() {
  // Initialize LCD
  lcd.init();
  lcd.backlight();

  // Initialize Serial connection
  Serial.begin(115200);
}

void loop() {
  // Read the potentiometer value
  int potValue = analogRead(POT_PIN);

  // Display the value on the LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Pot Value: ");
  lcd.print(potValue);

  // Send the value to the Serial Monitor
  Serial.println(potValue);

  // Wait 500ms
  delay(500);
}