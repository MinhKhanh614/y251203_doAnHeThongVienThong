#include "MK_SIMHandler.h"
#include "MK_SIM.h"

void taskSIMHandler(void *pvParameters)
{
    // Task này xử lý SIM module
    // Đọc incoming calls và gửi message tới main logic

    while (1)
    {
        // Kiểm tra xem có incoming call không
        if (flagSIM && incomingNumber.length() > 0)
        {
            // Gửi message tới main logic
            Serial.println("[SIMHANDLER] Sending MSG_PHONE_INCOMING: " + incomingNumber);
            Message msg(MSG_PHONE_INCOMING, incomingNumber);
            sendMessage(mainQueue, msg);
            Serial.println("[SIMHANDLER] Message sent: OK");

            // Reset flag
            flagSIM = false;
            flagDis = false;

            incomingNumber = "";
            SIM_SERIAL.println("ATH");
        }

        vTaskDelay(pdMS_TO_TICKS(100)); // Kiểm tra mỗi 100ms
    }
}
