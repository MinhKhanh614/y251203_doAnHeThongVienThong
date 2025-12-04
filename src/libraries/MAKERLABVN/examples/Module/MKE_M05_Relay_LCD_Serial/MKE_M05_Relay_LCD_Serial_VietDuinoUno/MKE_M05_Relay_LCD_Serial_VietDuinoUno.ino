// Add the library
#include <MKL_LiquidCrystal_I2C.h>

// LCD config
MKL_LiquidCrystal_I2C lcd(0x27, 16, 2);

// Define the pin for the relay
#define RELAY_PIN 10

void setup() {
  // Initialize LCD
  lcd.init();
  lcd.backlight();

  // Initialize Serial connection
  Serial.begin(115200);

  // Set the relay pin as output
  pinMode(RELAY_PIN, OUTPUT);
}

void loop() {
  // Turn the relay on
  digitalWrite(RELAY_PIN, HIGH);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Relay: ON");
  delay(1000);

  // Turn the relay off
  digitalWrite(RELAY_PIN, LOW);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Relay: OFF");
  delay(1000);
}