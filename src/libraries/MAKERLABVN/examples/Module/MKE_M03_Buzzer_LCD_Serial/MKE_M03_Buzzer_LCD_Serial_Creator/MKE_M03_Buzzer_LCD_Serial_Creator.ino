// Add the library
#include <MKL_LiquidCrystal_I2C.h>

// LCD config
MKL_LiquidCrystal_I2C lcd(0x27, 16, 2);

// Define the pin for the buzzer
#define BUZZER_PIN 10

void setup() {
  // Initialize LCD
  lcd.init();
  lcd.backlight();

  // Initialize Serial connection
  Serial.begin(115200);

  // Set the buzzer pin as output
  pinMode(BUZZER_PIN, OUTPUT);
}

void loop() {
  // Turn the buzzer on
  lcd.clear();
  digitalWrite(BUZZER_PIN, HIGH);
  lcd.setCursor(0, 0);
  lcd.print("Buzzer ON");
  delay(1000);

  // Turn the buzzer off
  lcd.clear();
  digitalWrite(BUZZER_PIN, LOW);
  lcd.setCursor(0, 0);
  lcd.print("Buzzer OFF");
  delay(1000);
}