#include "MK_KeypadHandler.h"
#include "MK_Keypad.h"

void taskKeypadHandler(void *pvParameters)
{
    // Task này đọc keypad và gửi message
    while (1)
    {
        // Biến này được set bởi ISR từ MK_Keypad.cpp
        extern volatile char key;

        char currentKey = key;

        // Chỉ xử lý khi có phím mới
        if (currentKey != '0' && currentKey != NO_KEY)
        {
            Message msg(MSG_KEYPAD_PRESSED, String(currentKey));
            sendMessage(mainQueue, msg);

            // Reset key
            key = '0';
        }

        vTaskDelay(pdMS_TO_TICKS(50)); // Đọc mỗi 50ms
    }
}
