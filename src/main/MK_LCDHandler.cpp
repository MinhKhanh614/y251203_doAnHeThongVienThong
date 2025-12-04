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
        // Format: "Line1|Line2" - adjust for 16x2 LCD
        int delimPos = msg.data.indexOf('|');
        if (delimPos != -1)
        {
            String newLine1 = msg.data.substring(0, delimPos);
            String newLine2 = msg.data.substring(delimPos + 1);

            // Truncate to 16 characters per line for 16x2 LCD and pad with spaces
            if (newLine1.length() > 16)
                newLine1 = newLine1.substring(0, 16);
            else
                while (newLine1.length() < 16)
                    newLine1 += " ";

            if (newLine2.length() > 16)
                newLine2 = newLine2.substring(0, 16);
            else
                while (newLine2.length() < 16)
                    newLine2 += " ";

            if (newLine1 != line1 || newLine2 != line2)
            {
                lcd.clear();
                lcd.setCursor(0, 0);
                lcd.print(newLine1);
                lcd.setCursor(0, 1);
                lcd.print(newLine2);
                line1 = newLine1;
                line2 = newLine2;
                Serial.println("[LCD] Display: '" + newLine1 + "' / '" + newLine2 + "'");
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
        lcd.setCursor(0, 1);
        lcd.print("Waiting...");
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
        lcd.print("SYS LOCKED");
        lcd.setCursor(0, 1);
        lcd.print("Wait:" + msg.data + "s");
        break;
    }

    case MSG_PHONE_AUTH_OK:
    {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("AUTH OK");
        lcd.setCursor(0, 1);
        // Truncate number to fit 16x2 LCD
        String displayNum = msg.data;
        if (displayNum.length() > 10)
            displayNum = displayNum.substring(0, 10);
        lcd.print(displayNum);
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
