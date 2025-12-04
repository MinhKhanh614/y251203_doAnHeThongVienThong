#include "MK_app.h"


void setup() {
Serial.begin(115200);
// Serial1.begin(115200, SERIAL_8N1, 13, 27);
app_run();

}

void loop() {
  // while (SIM_SERIAL.available())
  // {
  //   incomingNumber = readIncomingNumber();
  //   // char incomingNumber1 = SIM_SERIAL.read();
  //   // Serial.write(incomingNumber1);
  //   Serial.println(incomingNumber);

  //   flagSIM = true;
  // }

  // if(flagSIM)
  // {
  //   Serial.print(incomingNumber);
  //   incomingNumber = "";
  //   flagSIM = false;
  // }
  // if(Serial.available()) Serial1.write(Serial.read());
  // if(Serial1.available()) Serial.write(Serial1.read());
}




