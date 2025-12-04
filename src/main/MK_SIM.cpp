#include "MK_SIM.h"
#include <Arduino.h>

String simBuffer = "";
String incomingNumber = "";
bool flagSIM = false;

void Module_SIM_Init()
{
    // UART1 cho SIM7600 (TX=27, RX=13)
  SIM_SERIAL.begin(SIM_BAUDRATE, SIM_SERIAL_CONFIG, SIM_SERIAL_RX_PIN, SIM_SERIAL_TX_PIN);
  SIM_SERIAL.onReceive(simCallback);
  delay(10000);

  SIM_SERIAL.println("AT");
}

String readIncomingNumber() {
  String line = SIM_SERIAL.readStringUntil('\n'); // đọc một dòng từ SIM
  if (line.indexOf("+CLCC:") != -1) {
    int firstQuote = line.indexOf('"');
    int secondQuote = line.indexOf('"', firstQuote + 1);
    if (firstQuote != -1 && secondQuote != -1) {
      String number = line.substring(firstQuote + 1, secondQuote);
      return number;
    }
  }
  return "";
}
void IRAM_ATTR simCallback() {
  while (SIM_SERIAL.available())
  {
    incomingNumber = readIncomingNumber();
    flagSIM = true;
  }
}