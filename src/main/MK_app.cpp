#include "MK_app.h"

// Password đặt trước
String password = "1234567";
String input = "";

// Global state variables
enum SystemState
{
  STATE1 = 0,
  STATE2 = 1,
  STATE3 = 2,
  STATE4 = 3
};
volatile SystemState State = STATE1;
volatile int COUNT = 0;
volatile unsigned long lockTimeout = 0;
volatile unsigned long stateTimeout = 0;
const int LOCK_TIME = 30;    // 30 giây khóa
const int AUTH_TIMEOUT = 30; // 30 giây xác thực

// Hàm tạo chuỗi dấu '*'
String maskString(int length)
{
  String s = "";
  for (int i = 0; i < length; i++)
  {
    s += "*";
  }
  return s;
}
void module_init()
{
  keypad_init();
  Module_LCD_init();
  Module_SIM_Init();
}

// Task chạy logic chính của hệ thống
void taskSystemLogic(void *pvParameters)
{
  while (1)
  {
    handle_Process();
    vTaskDelay(pdMS_TO_TICKS(100)); // Cập nhật trạng thái mỗi 100ms
  }
}

void app_run()
{
  module_init();

  // Khởi động task chính
  xTaskCreate(
      taskSystemLogic,     // Function to implement the task
      "System Logic Task", // Name of the task
      4096,                // Stack size in words
      NULL,                // Task input parameter
      1,                   // Priority of the task
      NULL);               // Task handle
}

void handle_Process()
{
  switch (State)
  {
  case STATE1: // Nhập mật khẩu
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("ENTER PASSWORD");

    // Nhận input từ keypad
    char currentKey = key; // Từ MK_Keypad.cpp
    if (currentKey != '0' && currentKey != NO_KEY)
    {
      if (currentKey == '*') // Xóa ký tự cuối
      {
        if (input.length() > 0)
          input.remove(input.length() - 1);
      }
      else if (currentKey == '#') // Xác nhận password
      {
        if (input == password)
        {
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("PASSWORD OK");
          State = STATE2;
          stateTimeout = millis() + (AUTH_TIMEOUT * 1000);
          input = "";
          COUNT = 0;
          delay(1500);
        }
        else
        {
          COUNT = COUNT + 1;
          if (COUNT >= 3)
          {
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("SYSTEM LOCKED");
            State = STATE4;
            lockTimeout = millis() + (LOCK_TIME * 1000);
            delay(1500);
          }
          else
          {
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("WRONG PASSWORD");
            lcd.setCursor(0, 1);
            lcd.print("Try: " + String(3 - COUNT));
            input = "";
            delay(1500);
          }
        }
      }
      else // Thêm ký tự vào password
      {
        input += currentKey;
      }
    }

    // Hiển thị mật khẩu ẩn (dấu *)
    lcd.setCursor(0, 1);
    lcd.print("               "); // Clear line 2
    lcd.setCursor(0, 1);
    for (int i = 0; i < input.length(); i++)
      lcd.print("*");
    break;
  }

  case STATE2: // Xác thực qua số điện thoại
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("PHONE AUTH");
    lcd.setCursor(0, 1);
    lcd.print("Call to SIM");

    // Kiểm tra timeout
    if (millis() > stateTimeout)
    {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("AUTH TIMEOUT");
      State = STATE1;
      COUNT = 0;
      input = "";
      delay(1500);
      break;
    }

    // Kiểm tra incoming call
    if (flagSIM && incomingNumber.length() > 0)
    {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("AUTH OK");
      lcd.setCursor(0, 1);
      lcd.print("From: " + incomingNumber);
      State = STATE3;
      input = "";
      flagSIM = false;
      delay(2000);
    }
    break;
  }

  case STATE3: // Truy cập được cấp
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("ACCESS GRANTED");
    lcd.setCursor(0, 1);
    lcd.print("System Active");

    // TODO: Thực hiện các chức năng chính ở đây
    delay(500);
    break;
  }

  case STATE4: // Hệ thống bị khóa
  {
    long timeDiff = (long)(lockTimeout - millis());
    unsigned long remainingTime = (timeDiff > 0) ? (timeDiff / 1000) : 0;

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("SYSTEM LOCKED");
    lcd.setCursor(0, 1);
    lcd.print("Wait: " + String(remainingTime) + "s");

    // Kiểm tra timeout khóa
    if (millis() > lockTimeout)
    {
      COUNT = 0;
      State = STATE1;
      input = "";
      lcd.clear();
      delay(500);
    }
    break;
  }

  default:
    State = STATE1;
    break;
  }
}