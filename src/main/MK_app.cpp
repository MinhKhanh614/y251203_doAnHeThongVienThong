#include "MK_app.h"

// Khởi tạo queues
QueueHandle_t keypadQueue = NULL;
QueueHandle_t mainQueue = NULL;
QueueHandle_t displayQueue = NULL;

void module_init()
{
  keypad_init();
  Module_LCD_init();
  Module_SIM_Init();
}

void app_run()
{
  // Khởi tạo hardware modules
  module_init();

  // Tạo queues để trao đổi dữ liệu giữa tasks
  mainQueue = xQueueCreate(10, sizeof(Message));
  displayQueue = xQueueCreate(10, sizeof(Message));

  // Khởi động các tasks
  xTaskCreate(
      taskKeypadHandler, // Function
      "Keypad Handler",  // Name
      2048,              // Stack size
      NULL,              // Parameter
      2,                 // Priority (cao hơn)
      NULL);             // Handle

  xTaskCreate(
      taskLCDHandler, // Function
      "LCD Handler",  // Name
      3072,           // Stack size
      NULL,           // Parameter
      1,              // Priority
      NULL);          // Handle

  xTaskCreate(
      taskMainLogic, // Function
      "Main Logic",  // Name
      4096,          // Stack size (lớn nhất vì xử lý logic)
      NULL,          // Parameter
      2,             // Priority (cao hơn)
      NULL);         // Handle

  xTaskCreate(
      taskSIMHandler, // Function
      "SIM Handler",  // Name
      2048,           // Stack size
      NULL,           // Parameter
      2,              // Priority (cao hơn)
      NULL);          // Handle
}
