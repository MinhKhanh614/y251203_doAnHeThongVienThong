#include "PC_Control.h"
#include "MK_FLAG.h"

using namespace MK_flag;

// extern volatile bool flagSendSMS = false;
// extern volatile bool flagCheckIMEI = false;
// extern volatile bool flagGPS = false;
// extern volatile bool flagBalance = false;
// extern volatile bool flagAT = false;


void PC_Control_Init()
{
  PC_SERIAL.begin(PC_BAUDRATE);
    // Task PC Command Parser trên Core 1
  xTaskCreate(taskPC, "TaskPC", 8192, NULL, 1, NULL);

  // Task Manager trên Core 1
  // xTaskCreatePinnedToCore(taskManager, "TaskManager", 4096, NULL, 2, NULL, 1);
}

void mkSerial(Stream *in, Stream *out)
{
  if(in->available()) out->write(in->read());

  if(out->available()) in->write(out->read());
}

void taskPC(void *pvParameters) {
  while (1) {
    if (Serial.available()) {
      String cmd = Serial.readStringUntil('\n');
      cmd.trim();

      if (cmd == "IMEI") flagCheckIMEI = true;
      else if (cmd == "SMS") flagSendSMS = true;
      else if (cmd == "GPS") flagGPS = true;
      else if (cmd == "BALANCE") flagBalance = true;
      else if (cmd == "ATON") flagAT = true;
      else if (cmd == "ATOFF") flagAT = false;
    }
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

void taskManager(void *pvParameters)
{
  while(1)
  {
    if(flagAT) mkSerial(&Serial,&Serial1);
  }
    vTaskDelay(pdMS_TO_TICKS(1));
}

