#include "SIM.h"
#include "MK_FLAG.h"
// #include "PC_Control.h"
using namespace MK_flag;

// Định nghĩa các biến flag
volatile bool MK_flag::flagSendSMS = false;
volatile bool MK_flag::flagCheckIMEI = false;
volatile bool MK_flag::flagGPS = false;
volatile bool MK_flag::flagBalance = false;
volatile bool MK_flag::flagAT = false;

SIM7600 sim(SIM_SERIAL, SIM_BAUDRATE);

void SIM_Init(){

SIM_SERIAL.begin(SIM_BAUDRATE, SIM_SERIAL_CONFIG, SIM_SERIAL_RX_PIN, SIM_SERIAL_TX_PIN);

sim.begin();

xTaskCreate(SIM_task, "SIM_task", 16384, NULL, 1, NULL);

}

String decodeUCS2(const String &hex) {
  String out = "";
  for (int i = 0; i < hex.length() - 1; i += 2) {
    String part = hex.substring(i, i + 2);
    uint8_t byte = strtol(part.c_str(), NULL, 16);
    out += (char)byte;
  }
  return out;
}

void CheckBalance() {
  String resp = sim.sendAT("AT+CUSD=1,\"*101#\",15", 5000);
  
  if (resp.length() == 0) {
    Serial.println("Balance check timeout");
    return;
  }

  // Tìm response dạng: +CUSD: 0,"hex_string",72
  int start = resp.indexOf("+CUSD:");
  if (start == -1) {
    Serial.println("No CUSD response");
    return;
  }
  
  // Tìm dấu ngoặc kép đầu tiên sau +CUSD:
  int quoteStart = resp.indexOf("\"", start);
  int quoteEnd = resp.indexOf("\"", quoteStart + 1);
  
  if (quoteStart != -1 && quoteEnd != -1 && quoteEnd > quoteStart) {
    String hexStr = resp.substring(quoteStart + 1, quoteEnd);
    if (hexStr.length() > 0) {
      String decoded = decodeUCS2(hexStr);
      Serial.println("Balance: " + decoded);
    } else {
      Serial.println("Empty hex string");
    }
  } else {
    Serial.println("Invalid CUSD format");
  }
}


void SIM_task(void *pvParameters)
{
while (1) {
    if (flagCheckIMEI) {
      Serial.println("IMEI: " + sim.getIMEI());
      flagCheckIMEI = false;
    }

    if (flagSendSMS) {
      sim.sendSMS("+84983305910", "Hello from Arduino!");
      flagSendSMS = false;
    }

    if (flagGPS) {
      Serial.println("GPS Info: " + sim.gpsInfo());
      flagGPS = false;
    }

    if (flagBalance) {
      // Serial.println("Balance: " + CheckBalance());
      CheckBalance();
      flagBalance = false;
    }

    vTaskDelay(pdMS_TO_TICKS(100)); // nhường CPU
  }
}
