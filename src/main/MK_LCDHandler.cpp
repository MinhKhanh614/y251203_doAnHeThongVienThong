#include "MK_LCDHandler.h"
#include "MK_LCD.h"

// Biến để lưu trạng thái hiển thị hiện tại
static String line1 = "";
static String line2 = "";
static String passwordDisplay = "";
static unsigned long lastDisplayTime = 0;
static String lastDisplayRaw = "";

void displayMessage(const Message &msg)
{
    switch (msg.type)
    {
    case MSG_DISPLAY_UPDATE:
    {
        // debounce identical display updates within short interval
        unsigned long now = millis();
        if (msg.data == lastDisplayRaw && (now - lastDisplayTime) < 300)
            break;
        lastDisplayRaw = msg.data;
        lastDisplayTime = now;

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
                // Write line1 character-by-character to ensure correct placement
                for (int c = 0; c < 16; ++c)
                {
                    lcd.setCursor(c, 0);
                    lcd.write((uint8_t)newLine1.charAt(c));
                }
                // Write line2 character-by-character
                for (int c = 0; c < 16; ++c)
                {
                    lcd.setCursor(c, 1);
                    lcd.write((uint8_t)newLine2.charAt(c));
                }
                line1 = newLine1;
                line2 = newLine2;
                Serial.println("[LCD] Display: '" + newLine1 + "' / '" + newLine2 + "'");
            }
        }
        break;
    }

    case MSG_PASSWORD_INPUT:
    {
        // If we're currently showing PHONE AUTH, ignore transient password input
        if (line1.startsWith("PHONE AUTH"))
            break;
        // Hiển thị password (dưới dạng dấu *)
        passwordDisplay = msg.data;
        lcd.setCursor(0, 1);
        lcd.print("                "); // Clear full line 2 (16 spaces)
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
        // update cached display lines (pad to 16)
        line1 = String("PASSWORD OK") + String(' ', 6);
        line2 = String("Waiting...") + String(' ', 6);
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
        // cache lines padded
        line1 = String("WRONG PASSWORD") + String(' ', 3);
        {
            String t2 = String("Try: ") + msg.data;
            while (t2.length() < 16)
                t2 += ' ';
            line2 = t2;
        }
        delay(1500);
        break;
    }

    case MSG_SYSTEM_LOCKED:
    {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("SYS LOCKED");
        lcd.setCursor(0, 1);
        {
            String t = "Wait:" + msg.data + "s";
            while (t.length() < 16)
                t += ' ';
            lcd.print(t);
            line1 = String("SYS LOCKED") + String(' ', 6);
            line2 = t;
        }
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
        String t2 = displayNum;
        while (t2.length() < 16)
            t2 += ' ';
        lcd.print(t2);
        line1 = String("AUTH OK") + String(' ', 9);
        line2 = t2;
        delay(2000);
        break;
    }

    case MSG_AUTH_TIMEOUT:
    {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("AUTH TIMEOUT");
        line1 = String("AUTH TIMEOUT") + String(' ', 4);
        line2 = String(' ', 16);
        delay(1500);
        break;
    }

    case MSG_ACCESS_GRANTED:
    {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("ACCESS GRANTED");
        lcd.setCursor(0, 1);
        lcd.print("SYSTEM ACTIVE");
        line1 = String("ACCESS GRANTED") + String(' ', 3);
        {
            String t = "SYSTEM ACTIVE";
            while (t.length() < 16)
                t += ' ';
            line2 = t;
        }
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
