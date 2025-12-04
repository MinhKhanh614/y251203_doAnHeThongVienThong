#include "HardwareSerial.h"
#include "MK_app.h"

// Password đặt trước
String password = "1234";
String input = "";
String otp = "";

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

void app_run(){
  // keypad_init();
  // Module_LCD_init();
  Module_SIM_Init();
}

// void app_run()
// {
//   Serial.begin(115200);
//   xTaskCreatePinnedToCore(
//       task_app,   // Function to implement the task
//       "App Task", // Name of the task
//       8192,       // Stack size in words
//       NULL,       // Task input parameter
//       1,          // Priority of the task
//       NULL,       // Task handle
//       1);         // Core where the task should run
// }

// void task_app(void *pvParameters)
// {
//   lcd.clear();
//   lcd.print("Enter Password:");
//   String input = "";

//   while (true)
//   {
//     char key = customKeypad.getKey();

//     if (key)
//     {
//       // Nhấn '#' để xác nhận password
//       if (key == '#')
//       {
//         lcd.clear();
//         if (input == password)
//         {
//           lcd.print("Password OK");
//           delay(2000);

//           // Sinh OTP ngẫu nhiên 6 số
//           otp = String(random(100000, 999999));
//           sendSMS("+84983305910", "Your OTP is " + otp);
//           Serial.println("Your OTP is " + String(otp));
//           lcd.clear();
//           lcd.print("Enter OTP:");
//           String otpInput = getKeypadInput(false); // OTP hiển thị trực tiếp

//           lcd.clear();
//           if (otpInput == otp)
//           {
//             lcd.print("OK");
//           }
//           else
//           {
//             lcd.print("INVALID");
//           }
//           delay(2000);
//           lcd.clear();
//           lcd.print("Enter Password:");
//         }
//         else
//         {
//           lcd.print("INVALID");
//           delay(2000);
//           lcd.clear();
//           lcd.print("Enter Password:");
//         }
//         input = "";
//       }
//       // Nhấn '*' để xóa ký tự cuối
//       else if (key == '*')
//       {
//         if (input.length() > 0)
//         {
//           input.remove(input.length() - 1);
//         }
//         lcd.clear();
//         lcd.print("Enter Password:");
//         lcd.setCursor(0, 1);
//         lcd.print(maskString(input.length())); // hiển thị dấu *
//       }
//       // Nhập ký tự bình thường
//       else
//       {
//         input += key;
//         lcd.setCursor(0, 1);
//         lcd.print(maskString(input.length())); // hiển thị dấu *
//       }
//     }
//     vTaskDelay(dMS_TO_TICKS(1));
//   }
// }