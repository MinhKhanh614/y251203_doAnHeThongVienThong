#include "MK_LCDHandler.h"
#include "MK_LCD.h"

// Biến để lưu trạng thái hiển thị hiện tại
static String line1 = "";
static String line2 = "";
static String passwordDisplay = "";

void displayMessage(const Message &msg)
{
    switch (msg.type)
    {
    case MSG_DISPLAY_UPDATE:
    {
        // Format: "Line1|Line2"
        int delimPos = msg.data.indexOf('|');
        if (delimPos != -1)
        {
            String newLine1 = msg.data.substring(0, delimPos);
            String newLine2 = msg.data.substring(delimPos + 1);

            if (newLine1 != line1 || newLine2 != line2)
            {
                lcd.clear();
                lcd.setCursor(0, 0);
                lcd.print(newLine1);
                if (newLine2.length() > 0)
                {
                    lcd.setCursor(0, 1);
                    lcd.print(newLine2);
                }
                line1 = newLine1;
                line2 = newLine2;
            }
        }
        break;
    }

    case MSG_PASSWORD_INPUT:
    {
        // Hiển thị password (dưới dạng dấu *)
        passwordDisplay = msg.data;
        lcd.setCursor(0, 1);
        lcd.print("               "); // Clear line 2
        lcd.setCursor(0, 1);
        for (int i = 0; i < passwordDisplay.length(); i++)
            lcd.print("*");
        break;
    }

    case MSG_PASSWORD_CORRECT:
    {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("PASSWORD OK");
        delay(1500);
        break;
    }

    case MSG_PASSWORD_WRONG:
    {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("WRONG PASSWORD");
        lcd.setCursor(0, 1);
        lcd.print("Try: " + msg.data);
        delay(1500);
        break;
    }

    case MSG_SYSTEM_LOCKED:
    {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("SYSTEM LOCKED");
        lcd.setCursor(0, 1);
        lcd.print("Wait: " + msg.data + "s");
        break;
    }

    case MSG_PHONE_AUTH_OK:
    {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("AUTH OK");
        lcd.setCursor(0, 1);
        lcd.print("From: " + msg.data);
        delay(2000);
        break;
    }

    case MSG_AUTH_TIMEOUT:
    {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("AUTH TIMEOUT");
        delay(1500);
        break;
    }

    case MSG_ACCESS_GRANTED:
    {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("ACCESS GRANTED");
        lcd.setCursor(0, 1);
        lcd.print("System Active");
        delay(500);
        break;
    }

    default:
        break;
    }
}

void taskLCDHandler(void *pvParameters)
{
    // Task này nhận message từ main queue và hiển thị
    Message msg;

    // Khởi tạo
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("ENTER PASSWORD");
    line1 = "ENTER PASSWORD";
    line2 = "";

    while (1)
    {
        // Nhận message với timeout 100ms
        if (receiveMessage(displayQueue, msg, 100))
        {
            displayMessage(msg);
        }

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}
