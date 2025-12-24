# Giải thích cách hoạt động và vai trò của các đoạn code trong hệ thống viễn thông

## Tổng quan hệ thống

Hệ thống này là một ứng dụng Arduino sử dụng module SIM7600, keypad 4x4, và LCD I2C để tạo một "bộ khóa điện tử với xác thực 2 yếu tố". Người dùng nhập mật khẩu qua keypad, sau đó xác thực bằng cuộc gọi đến từ số điện thoại được phép. Hệ thống sử dụng FreeRTOS để chạy đa nhiệm và state machine để quản lý logic.

## Các kỹ thuật lập trình được sử dụng

1. **Embedded Programming với Arduino**: Sử dụng Arduino framework cho I/O, Serial communication, và timers.
2. **Real-Time Operating System (FreeRTOS)**: Multi-tasking với xTaskCreate, queues (xQueueCreate), và synchronization primitives.
3. **State Machine**: Quản lý logic với enum SystemState và switch-case trong taskMainLogic.
4. **Message Passing**: Giao tiếp bất đồng bộ giữa tasks qua FreeRTOS queues và struct Message.
5. **Interrupt/Callback Handling**: UART callbacks (onReceive) cho SIM module, ISR-like cho keypad.
6. **String Manipulation**: Xử lý phone numbers với String class, normalize, và parsing.
7. **Timeout Management**: Sử dụng millis() cho non-blocking delays và timeouts.
8. **Modular Programming**: Chia code thành modules (MK_app, MK_Keypad, MK_LCD, etc.) với header files.
9. **Debouncing và Filtering**: Tránh duplicate inputs trong LCD handler và keypad reading.
10. **AT Commands**: Giao tiếp với SIM7600 qua UART để xử lý cuộc gọi.

## Giải thích chi tiết từng đoạn code

### 1. main.ino - Điểm khởi đầu

```cpp
#include "MK_app.h"

void setup() {
  Serial.begin(115200);  // Khởi tạo Serial cho debug
  app_run();             // Khởi động ứng dụng
}

void loop() {
  // Loop trống - toàn bộ logic chạy trên FreeRTOS tasks
}
```

**Cách hoạt động**: Arduino setup() chạy một lần để khởi tạo. app_run() tạo tasks và queues. Loop() không làm gì vì tasks xử lý mọi thứ song song.
**Vai trò**: Entry point, đảm bảo hệ thống khởi động đúng thứ tự.

### 2. MK_app.cpp/h - Quản lý ứng dụng

```cpp
void module_init() {
  keypad_init();      // Khởi tạo keypad
  Module_LCD_init();  // Khởi tạo LCD
  Module_SIM_Init();  // Khởi tạo SIM7600
}

void app_run() {
  module_init();
  mainQueue = xQueueCreate(10, sizeof(Message));    // Queue cho main logic
  displayQueue = xQueueCreate(10, sizeof(Message)); // Queue cho LCD

  // Tạo 4 tasks với stack size và priority khác nhau
  xTaskCreate(taskKeypadHandler, "Keypad Handler", 2048, NULL, 2, NULL);
  xTaskCreate(taskLCDHandler, "LCD Handler", 3072, NULL, 1, NULL);
  xTaskCreate(taskMainLogic, "Main Logic", 4096, NULL, 2, NULL);
  xTaskCreate(taskSIMHandler, "SIM Handler", 2048, NULL, 2, NULL);
}
```

**Cách hoạt động**: Khởi tạo hardware, tạo queues để tasks giao tiếp, start tasks với FreeRTOS.
**Vai trò**: Orchestrator, đảm bảo tất cả components hoạt động đồng bộ.

### 3. MK_MainLogic.cpp/h - Logic chính

```cpp
enum SystemState { STATE1=0, STATE2=1, STATE3=2, STATE4=3 };

struct SystemContext {
  SystemState currentState;
  String password;  // Mật khẩu mặc định "1234567"
  String passwordInput;
  int attemptCount;
  unsigned long lockTimeout, authTimeout;
  String deviceNumber, allowedNumber;  // Danh sách số được phép
};

void taskMainLogic(void *pvParameters) {
  while(1) {
    // Nhận message từ queue (timeout 100ms)
    if (receiveMessage(mainQueue, msg, 100)) {
      switch(msg.type) {
        case MSG_KEYPAD_PRESSED: handlePasswordInput(key); break;
        case MSG_PHONE_INCOMING: /* Kiểm tra số gọi */ break;
      }
    }
    // Xử lý state mỗi 500ms
    switch(sysCtx.currentState) {
      case STATE1: handleState1(); break;
      // ... các state khác
    }
  }
}
```

**Cách hoạt động**: Task chạy vòng lặp, nhận messages, xử lý input, và update state dựa trên thời gian.
**Vai trò**: Bộ não, điều phối toàn bộ flow: nhập PW → xác thực → access/khóa.

### 4. MK_Keypad.cpp/h & MK_KeypadHandler.cpp - Xử lý input

```cpp
// MK_Keypad.cpp
volatile char key = '0';
void taskREAD_KEYPAD() {
  char k = customKeypad.getKey();
  if (k != NO_KEY) key = k;
}

// MK_KeypadHandler.cpp
void taskKeypadHandler() {
  if (key != '0') {
    sendMessage(mainQueue, Message(MSG_KEYPAD_PRESSED, String(key)));
    key = '0';
  }
}
```

**Cách hoạt động**: TaskREAD_KEYPAD scan keypad liên tục. TaskKeypadHandler gửi message khi có input.
**Vai trò**: Đọc và truyền input từ user đến logic chính.

### 5. MK_LCD.cpp/h & MK_LCDHandler.cpp - Hiển thị

```cpp
// MK_LCD.cpp
void Module_LCD_init() { lcd.init(); lcd.backlight(); }

// MK_LCDHandler.cpp
void taskLCDHandler() {
  Message msg;
  if (receiveMessage(displayQueue, msg, 0)) {
    switch(msg.type) {
      case MSG_DISPLAY_UPDATE: /* Parse "Line1|Line2" và hiển thị */ break;
      case MSG_PASSWORD_INPUT: /* Hiển thị * cho password */ break;
    }
  }
}
```

**Cách hoạt động**: Nhận messages từ displayQueue, update LCD với text phù hợp.
**Vai trò**: Giao diện user, hiển thị trạng thái và prompts.

### 6. MK_SIM.cpp/h & MK_SIMHandler.cpp - Xử lý cuộc gọi

```cpp
// MK_SIM.cpp
void simCallback() {
  incomingNumber = readIncomingNumber();  // Parse từ AT command
  flagSIM = true;
}

// MK_SIMHandler.cpp
void taskSIMHandler() {
  if (flagSIM) {
    sendMessage(mainQueue, Message(MSG_PHONE_INCOMING, incomingNumber));
    flagSIM = false;
  }
}
```

**Cách hoạt động**: UART callback đọc data từ SIM7600, extract số điện thoại. Task gửi message.
**Vai trò**: Phát hiện và truyền thông tin cuộc gọi đến logic chính.

### 7. MK_Message.cpp/h - Hệ thống message

```cpp
enum MessageType { MSG_KEYPAD_PRESSED, MSG_PHONE_INCOMING, MSG_DISPLAY_UPDATE, ... };
struct Message { MessageType type; String data; unsigned long timestamp; };

void sendMessage(QueueHandle_t queue, const Message &msg) {
  xQueueSend(queue, &msg, portMAX_DELAY);
}

bool receiveMessage(QueueHandle_t queue, Message &msg, int timeoutMs) {
  return xQueueReceive(queue, &msg, pdMS_TO_TICKS(timeoutMs)) == pdTRUE;
}
```

**Cách hoạt động**: Đóng gói data thành Message, gửi/nhận qua queues.
**Vai trò**: Giao tiếp an toàn giữa tasks, tránh race conditions.

## Luồng hoạt động tổng thể

1. **Khởi động**: setup() → app_run() → tạo tasks và queues.
2. **Nhập PW (STATE1)**: Keypad → MainLogic → LCD hiển thị.
3. **Xác thực (STATE2)**: SIM detect call → MainLogic kiểm tra số → LCD update.
4. **Access/Khóa**: Dựa trên kết quả, chuyển state và hiển thị.
5. **Reset**: Sau timeout, quay về STATE1.

Hệ thống đảm bảo bảo mật với 2FA (password + phone auth) và xử lý lỗi (wrong PW, timeout, invalid caller).

## Chi tiết hàm, cấu trúc và logic hoạt động

### Cấu trúc dữ liệu chính

#### 1. Enum SystemState (trong MK_MainLogic.cpp)

```cpp
enum SystemState {
    STATE1 = 0, // Nhập password
    STATE2 = 1, // Xác thực phone
    STATE3 = 2, // Truy cập được cấp
    STATE4 = 3  // Hệ thống bị khóa
};
```

**Mô tả**: Định nghĩa 4 trạng thái của state machine. Logic chuyển đổi dựa trên events (keypad, phone, timeout).

#### 2. Struct SystemContext (trong MK_MainLogic.cpp)

```cpp
struct SystemContext {
    SystemState currentState;        // Trạng thái hiện tại
    String password;                 // Mật khẩu mặc định ("1234567")
    String passwordInput;            // Input đang nhập
    int attemptCount;                // Số lần thử sai
    unsigned long lockTimeout;       // Thời điểm unlock (millis)
    unsigned long authTimeout;       // Timeout xác thực (millis)
    unsigned long wrongPasswordDisplayTime; // Delay hiển thị lỗi
    String deviceNumber;             // Số của thiết bị
    String allowedNumber;            // Danh sách số được phép (comma-separated)
    const int MAX_ATTEMPTS = 3;      // Max attempts
    const int LOCK_TIME = 30000;     // Thời gian khóa (30s)
    const int AUTH_TIMEOUT = 30000;  // Timeout xác thực (30s)
    const int WRONG_PASSWORD_DISPLAY_TIME = 1500; // Delay lỗi (1.5s)
};
```

**Mô tả**: Lưu toàn bộ trạng thái hệ thống. Khởi tạo với giá trị mặc định.

#### 3. Struct Message (trong MK_Message.h)

```cpp
struct Message {
    MessageType type;        // Loại message
    String data;             // Dữ liệu kèm theo
    unsigned long timestamp; // Thời gian gửi

    Message();                          // Constructor mặc định
    Message(MessageType t);             // Constructor với type
    Message(MessageType t, String d);   // Constructor với type và data
};
```

**Mô tả**: Đóng gói thông tin truyền giữa tasks. Timestamp để debug.

#### 4. Enum MessageType (trong MK_Message.h)

```cpp
enum MessageType {
    MSG_KEYPAD_PRESSED,    // Phím nhấn (data: key char)
    MSG_PASSWORD_INPUT,    // Input PW (data: current input)
    MSG_PASSWORD_VERIFY,   // Xác thực PW
    MSG_PASSWORD_CORRECT,  // PW đúng
    MSG_PASSWORD_WRONG,    // PW sai (data: attempts left)
    MSG_SYSTEM_LOCKED,     // Khóa hệ thống (data: seconds left)
    MSG_PHONE_INCOMING,    // Cuộc gọi đến (data: số gọi)
    MSG_PHONE_AUTH_OK,     // Xác thực phone OK
    MSG_AUTH_TIMEOUT,      // Timeout xác thực
    MSG_STATE_CHANGED,     // Trạng thái thay đổi
    MSG_DISPLAY_UPDATE,    // Cập nhật LCD (data: "Line1|Line2")
    MSG_ACCESS_GRANTED,    // Truy cập OK
    MSG_SYSTEM_RESET       // Reset hệ thống
};
```

**Mô tả**: Định nghĩa các loại message để tasks hiểu và xử lý.

### Các hàm chính và logic

#### 1. Functions trong MK_app.cpp

- **void module_init()**: Không có parameters. Gọi keypad_init(), Module_LCD_init(), Module_SIM_Init(). **Logic**: Khởi tạo tất cả hardware modules.
- **void app_run()**: Không có parameters. Tạo queues, tạo 4 tasks với xTaskCreate. **Parameters cho xTaskCreate**: function pointer, name (string), stack size (bytes), parameter (NULL), priority (1-2), handle (NULL).

#### 2. Functions trong MK_MainLogic.cpp

- **void taskMainLogic(void *pvParameters)**: Parameter: pvParameters (NULL). **Logic**: Vòng lặp while(1), nhận message từ mainQueue (timeout 100ms), xử lý switch(msg.type), update state mỗi 500ms.
- **void handlePasswordInput(char key)**: Parameter: key (char từ keypad). **Logic**: Nếu '#' xác thực PW, '*' xóa, else thêm. Nếu sai tăng attempt, nếu >=3 khóa. Gửi message đến displayQueue.
- **void handleState1()**: Không parameters. **Logic**: Nếu delay xong, gửi MSG_DISPLAY_UPDATE "ENTER PASSWORD|".
- **void handleState2()**: Không parameters. **Logic**: Kiểm tra authTimeout, nếu hết gửi MSG_AUTH_TIMEOUT, delay 2s rồi reset về STATE1.
- **void handleState3()**: Không parameters. **Logic**: Hiển thị access granted, sau 5s reset về STATE1.
- **void handleState4()**: Không parameters. **Logic**: Hiển thị thời gian còn lại, nếu hết timeout reset về STATE1.
- **String normalizePhoneNumber(const String &in)**: Parameter: in (String số điện thoại). **Return**: String normalized (chỉ số và +). **Logic**: Lọc ký tự không phải số/+.
- **void addAllowedNumber(const String &num)**: Parameter: num (String số). **Logic**: Normalize, kiểm tra duplicate, thêm vào sysCtx.allowedNumber (comma-separated).
- **bool isAllowedNumber(const String &num)**: Parameter: num (String). **Return**: bool. **Logic**: Normalize và tìm trong danh sách.

#### 3. Functions trong MK_Keypad.cpp

- **void keypad_init()**: Không parameters. **Logic**: Tạo task taskREAD_KEYPAD với xTaskCreate.
- **void taskREAD_KEYPAD(void* pvParameters)**: Parameter: pvParameters (NULL). **Logic**: Vòng lặp getKey() từ Keypad object, set volatile key nếu PRESSED.

#### 4. Functions trong MK_KeypadHandler.cpp

- **void taskKeypadHandler(void *pvParameters)**: Parameter: pvParameters (NULL). **Logic**: Kiểm tra volatile key, nếu != '0' gửi MSG_KEYPAD_PRESSED, reset key.

#### 5. Functions trong MK_LCD.cpp

- **void Module_LCD_init()**: Không parameters. **Logic**: lcd.init(), backlight(). Nếu KEYPAD_DEBUG, tạo task taskMonitor_LCD.
- **void taskMonitor_LCD(void* pvParameters)**: Parameter: pvParameters (NULL). **Logic**: Debug, in key lên LCD và Serial.

#### 6. Functions trong MK_LCDHandler.cpp

- **void taskLCDHandler(void *pvParameters)**: Parameter: pvParameters (NULL). **Logic**: Nhận message từ displayQueue, xử lý switch(msg.type) để update LCD (clear, setCursor, print).
- **void displayMessage(const Message &msg)**: Parameter: msg (const reference). **Logic**: Parse data, truncate, pad spaces, write to LCD.

#### 7. Functions trong MK_SIM.cpp

- **void Module_SIM_Init()**: Không parameters. **Logic**: Serial1.begin() với pins, baud, config, set callback, delay 10s, gửi "AT".
- **String readIncomingNumber()**: Không parameters. **Return**: String số điện thoại. **Logic**: Đọc line từ Serial, parse "+CLCC:" để extract số trong quotes.
- **void simCallback()**: Không parameters (ISR). **Logic**: Đọc available data, gọi readIncomingNumber(), set flagSIM nếu có số.

#### 8. Functions trong MK_SIMHandler.cpp

- **void taskSIMHandler(void *pvParameters)**: Parameter: pvParameters (NULL). **Logic**: Nếu flagSIM, gửi MSG_PHONE_INCOMING với incomingNumber, reset flag.

#### 9. Functions trong MK_Message.cpp

- **void sendMessage(QueueHandle_t queue, const Message &msg)**: Parameters: queue (handle), msg (const reference). **Logic**: xQueueSend với portMAX_DELAY.
- **bool receiveMessage(QueueHandle_t queue, Message &msg, int timeoutMs)**: Parameters: queue, msg (reference), timeoutMs. **Return**: bool success. **Logic**: xQueueReceive với timeout converted to ticks.

### Logic hoạt động tổng thể

1. **Initialization**: app_run() tạo queues và tasks.
2. **Keypad Input**: taskREAD_KEYPAD → volatile key → taskKeypadHandler → MSG_KEYPAD_PRESSED → taskMainLogic → handlePasswordInput.
3. **SIM Input**: UART callback → incomingNumber, flagSIM → taskSIMHandler → MSG_PHONE_INCOMING → taskMainLogic.
4. **State Transitions**: Dựa trên messages và timeouts, chuyển state và gửi MSG_DISPLAY_UPDATE → taskLCDHandler → update LCD.
5. **Security**: Validate PW, check allowed numbers, handle attempts và locks.
6. **Non-blocking**: Tất cả tasks dùng vTaskDelay() để yield CPU, tránh blocking.

## Liệt kê dòng code và giải thích RTOS/Queue

### Dòng code được giải thích (từ các file chính)

#### Từ MK_app.cpp (dòng 10-35)

```cpp
void app_run()
{
  // Khởi tạo hardware modules
  module_init();

  // Tạo queues để trao đổi dữ liệu giữa tasks
  mainQueue = xQueueCreate(10, sizeof(Message));  // Dòng 16
  displayQueue = xQueueCreate(10, sizeof(Message)); // Dòng 17

  // Khởi động các tasks
  xTaskCreate(
      taskKeypadHandler, // Function
      "Keypad Handler",  // Name
      2048,              // Stack size
      NULL,              // Parameter
      2,                 // Priority (cao hơn)
      NULL);             // Handle  // Dòng 20-27

  xTaskCreate(
      taskLCDHandler, // Function
      "LCD Handler",  // Name
      3072,           // Stack size
      NULL,           // Parameter
      1,              // Priority
      NULL);          // Handle  // Dòng 29-36

  xTaskCreate(
      taskMainLogic, // Function
      "Main Logic",  // Name
      4096,          // Stack size (lớn nhất vì xử lý logic)
      NULL,          // Parameter
      2,              // Priority (cao hơn)
      NULL);         // Handle  // Dòng 38-45

  xTaskCreate(
      taskSIMHandler, // Function
      "SIM Handler",  // Name
      2048,           // Stack size
      NULL,           // Parameter
      2,              // Priority (cao hơn)
      NULL);          // Handle  // Dòng 47-54
}
```

#### Từ MK_MainLogic.cpp (dòng 298-330)

```cpp
void taskMainLogic(void *pvParameters)
{
    Message msg;
    unsigned long lastUpdateTime = millis();
    const int UPDATE_INTERVAL = 500; // Cập nhật trạng thái mỗi 500ms

    while (1)
    {
        // Nhận message từ keypad/phone (không block - timeout 100ms để đủ thời gian nhận)
        if (receiveMessage(mainQueue, msg, 100))  // Dòng 307
        {
            Serial.println("[MAIN] Received message type: " + String(msg.type) + ", currentState: " + String(sysCtx.currentState));
            switch (msg.type)
            {
            case MSG_KEYPAD_PRESSED:
            {
                char key = msg.data[0];
                // Chỉ xử lý nhập mật khẩu khi đang ở STATE1
                if (sysCtx.currentState == STATE1)
                {
                    handlePasswordInput(key);
                }
                else
                {
                    // Nếu muốn, có thể gửi thông báo rằng hệ thống đang chờ cuộc gọi
                    // Message notify(MSG_DISPLAY_UPDATE, "WAIT CALL|");
                    // sendMessage(displayQueue, notify);
                }
                break;
            }

            case MSG_PHONE_INCOMING:
            {
                // Xử lý incoming call - chỉ khi đang chờ xác thực
                if (sysCtx.currentState == STATE2)
                {
                    // Kiểm tra payload
                    String incomingNorm = normalizePhoneNumber(msg.data);
                    Serial.println("[MAIN] Incoming call from: " + msg.data);
                    Serial.println("[MAIN] Normalized: " + incomingNorm);
                    Serial.println("[MAIN] Allowed list: " + sysCtx.allowedNumber);

                    // Use isAllowedNumber to check against comma-separated list
                    if (incomingNorm.length() > 0 && isAllowedNumber(incomingNorm))
                    {
                        Serial.println("[MAIN] AUTH OK - Caller matched!");
                        Message displayMsg(MSG_PHONE_AUTH_OK, incomingNorm);
                        sendMessage(displayQueue, displayMsg);  // Dòng 340

                        sysCtx.currentState = STATE3;
                        // reset auth timeout
                        sysCtx.authTimeout = 0;
                    }
                    else
                    {
                        // Caller không khớp; có thể log hoặc thông báo
                        Serial.println("[MAIN] AUTH FAILED - Caller not in allowed list");
                        Message badCaller(MSG_PASSWORD_WRONG, "Invalid caller");
                        sendMessage(displayQueue, badCaller);  // Dòng 351
                    }
                }
                break;
            }

            default:
                break;
            }
        }

        // Xử lý logic theo state mỗi UPDATE_INTERVAL
        if (millis() - lastUpdateTime >= UPDATE_INTERVAL)
        {
            switch (sysCtx.currentState)
            {
            case STATE1:
                handleState1();
                break;
            case STATE2:
                handleState2();
                break;
            case STATE3:
                handleState3();
                break;
            case STATE4:
                handleState4();
                break;
            default:
                break;
            }

            lastUpdateTime = millis();
        }

        vTaskDelay(pdMS_TO_TICKS(50));  // Dòng 378
    }
}
```

#### Từ MK_Message.cpp (dòng 10-20)

```cpp
void sendMessage(QueueHandle_t queue, const Message &msg)
{
    if (queue != NULL)
    {
        xQueueSend(queue, &msg, portMAX_DELAY);  // Dòng 14
    }
}

bool receiveMessage(QueueHandle_t queue, Message &msg, int timeoutMs)
{
    if (queue == NULL)
        return false;

    TickType_t timeout = (timeoutMs == 0) ? 0 : pdMS_TO_TICKS(timeoutMs);
    return xQueueReceive(queue, &msg, timeout) == pdTRUE;  // Dòng 23
}
```

#### Từ MK_KeypadHandler.cpp (dòng 10-25)

```cpp
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
            sendMessage(mainQueue, msg);  // Dòng 22

            // Reset key
            key = '0';
        }

        vTaskDelay(pdMS_TO_TICKS(50));  // Dòng 27
    }
}
```

### Giải thích kỹ các hàm RTOS và Queue

#### 1. xTaskCreate (từ FreeRTOS)

- **Cú pháp**: BaseType_t xTaskCreate(TaskFunction_t pvTaskCode, const char *const pcName, const uint16_t usStackDepth, void* const pvParameters, UBaseType_t uxPriority, TaskHandle_t * const pxCreatedTask)
- **Parameters**:
  - pvTaskCode: Con trỏ hàm task (vd: taskMainLogic)
  - pcName: Tên task (string, vd: "Main Logic")
  - usStackDepth: Kích thước stack (words, 1 word = 4 bytes trên ESP32)
  - pvParameters: Tham số truyền vào task (thường NULL)
  - uxPriority: Độ ưu tiên (0 thấp nhất, configMAX_PRIORITIES-1 cao nhất)
  - pxCreatedTask: Handle để quản lý task (NULL nếu không cần)
- **Logic**: Tạo task mới chạy song song. Task bắt đầu từ pvTaskCode với pvParameters.
- **Tại sao dùng**: Cho phép multi-tasking, mỗi module (keypad, LCD, SIM) chạy riêng.

#### 2. xQueueCreate (từ FreeRTOS)

- **Cú pháp**: QueueHandle_t xQueueCreate(UBaseType_t uxQueueLength, UBaseType_t uxItemSize)
- **Parameters**:
  - uxQueueLength: Số items tối đa trong queue (vd: 10)
  - uxItemSize: Kích thước mỗi item (vd: sizeof(Message))
- **Return**: Handle của queue
- **Logic**: Tạo queue FIFO để lưu trữ messages. Blocks nếu full/empty tùy mode.
- **Tại sao dùng**: Giao tiếp an toàn giữa tasks, tránh race conditions.

#### 3. xQueueSend (trong sendMessage)

- **Cú pháp**: BaseType_t xQueueSend(QueueHandle_t xQueue, const void * pvItemToQueue, TickType_t xTicksToWait)
- **Parameters**:
  - xQueue: Handle queue
  - pvItemToQueue: Con trỏ item gửi (&msg)
  - xTicksToWait: Thời gian chờ nếu queue full (portMAX_DELAY = vô hạn)
- **Logic**: Copy item vào queue. Nếu full, chờ hoặc fail.
- **Tại sao dùng**: Gửi message bất đồng bộ, task không block lâu.

#### 4. xQueueReceive (trong receiveMessage)

- **Cú pháp**: BaseType_t xQueueReceive(QueueHandle_t xQueue, void * const pvBuffer, TickType_t xTicksToWait)
- **Parameters**:
  - xQueue: Handle queue
  - pvBuffer: Buffer nhận (&msg)
  - xTicksToWait: Timeout (0 = không chờ, pdMS_TO_TICKS(timeoutMs) = chờ ms)
- **Logic**: Copy item từ queue vào buffer. Nếu empty, chờ timeout.
- **Tại sao dùng**: Nhận message với timeout, tránh block vĩnh viễn.

#### 5. vTaskDelay (từ FreeRTOS)

- **Cú pháp**: void vTaskDelay(const TickType_t xTicksToDelay)
- **Parameters**: xTicksToDelay: Số ticks chờ (pdMS_TO_TICKS(ms) chuyển ms sang ticks)
- **Logic**: Task ngủ xTicksToDelay, yield CPU cho tasks khác.
- **Tại sao dùng**: Non-blocking delays, đảm bảo responsiveness.

### Tại sao chọn bộ nhớ và stack như vậy?

#### Stack Sizes

- **2048 bytes (KeypadHandler, SIMHandler)**: Tasks đơn giản (chỉ gửi/nhận message, ít biến local). ESP32 stack unit là 4 bytes, vậy ~512 words. Đủ cho String ngắn và Message.
- **3072 bytes (LCDHandler)**: Cần nhiều hơn cho LCD operations (setCursor, print strings dài), và xử lý parsing "Line1|Line2". Tránh stack overflow khi update LCD.
- **4096 bytes (MainLogic)**: Phức tạp nhất - xử lý state machine, String manipulation (normalize phone), nhiều if/switch, và calls đến functions. Cần stack lớn để tránh corruption.

#### Queue Length (10 items)

- **Lý do**: Đủ buffer cho events (keypad presses, phone calls). Không quá lớn để tiết kiệm RAM. Nếu overflow, task gửi sẽ block (portMAX_DELAY), đảm bảo không mất data.

#### Priorities

- **Priority 2 (Keypad, MainLogic, SIM)**: Cao hơn LCD (1) vì input/output quan trọng hơn display. MainLogic cần priority cao để xử lý logic kịp thời.
- **Tại sao**: Đảm bảo responsive cho user input và phone events.

#### Bộ nhớ tổng thể

- ESP32 có ~320KB RAM. FreeRTOS + queues + stacks ~50-100KB, còn lại cho heap. Chọn stack vừa đủ để tránh waste, nhưng đủ cho worst-case (String dài, recursion nhẹ).

Những lựa chọn này dựa trên testing và best practices cho embedded RTOS, đảm bảo stability và performance.

## Giải thích cặn kẽ toàn bộ mã hệ thống

Dưới đây là giải thích chi tiết từng file/module trong hệ thống, dựa trên việc đọc lại toàn bộ code. Hệ thống sử dụng Arduino framework trên ESP32 với FreeRTOS để quản lý tasks và queues.

### 1. main.ino - Điểm khởi đầu

**Code chính**:

```cpp
#include "MK_app.h"

void setup() {
  Serial.begin(115200);
  app_run();
}

void loop() {
  // Empty loop
}
```

**Giải thích cặn kẽ**:

- `setup()`: Chạy một lần khi khởi động. Khởi tạo Serial cho debug (baud 115200), gọi `app_run()` để start hệ thống.
- `loop()`: Trống vì logic chạy trên FreeRTOS tasks, không cần polling loop.
- **Vai trò**: Entry point, đảm bảo hệ thống boot đúng thứ tự. Không xử lý logic trực tiếp.

### 2. MK_app.h - Header cho app

**Code chính**:

```cpp
#pragma once
#include "MK_LCD.h"
// ... includes cho các modules
void app_run();
void module_init();
```

**Giải thích**: Header file khai báo prototypes. Bao gồm tất cả headers cần thiết để tránh conflicts.

### 3. MK_app.cpp - Quản lý ứng dụng

**Code chính** (đã liệt kê ở trên).
**Giải thích cặn kẽ**:

- `module_init()`: Gọi init functions của keypad, LCD, SIM để setup hardware.
- `app_run()`: Tạo 2 queues (mainQueue: 10 items cho logic, displayQueue: 10 items cho LCD). Tạo 4 tasks với xTaskCreate:
  - taskKeypadHandler: Stack 2048, Priority 2
  - taskLCDHandler: Stack 3072, Priority 1
  - taskMainLogic: Stack 4096, Priority 2
  - taskSIMHandler: Stack 2048, Priority 2
- **Logic**: Đảm bảo multi-tasking. Queues cho IPC (Inter-Process Communication).
- **Vai trò**: Orchestrator, khởi tạo và start toàn hệ thống.

### 4. MK_Message.h - Định nghĩa Message system

**Code chính** (đã liệt kê).
**Giải thích cặn kẽ**:

- Enum MessageType: 13 types cho events (keypad, phone, display, etc.).
- Struct Message: type (MessageType), data (String), timestamp (millis()).
- Constructors: Dễ tạo Message với type/data.
- Extern queues: keypadQueue (không dùng), mainQueue, displayQueue.
- Prototypes: sendMessage, receiveMessage.
- **Logic**: Chuẩn hóa giao tiếp. Timestamp để debug timing.
- **Vai trò**: Core của IPC, đảm bảo tasks trao đổi data an toàn.

### 5. MK_Message.cpp - Implement Message functions

**Code chính** (đã liệt kê).
**Giải thích cặn kẽ**:

- `sendMessage()`: Gọi xQueueSend với portMAX_DELAY (chờ vô hạn nếu queue full).
- `receiveMessage()`: Gọi xQueueReceive với timeout (0 hoặc ms). Return bool success.
- **Logic**: Wrapper cho FreeRTOS queues, handle NULL checks.
- **Vai trò**: Abstract queue operations, dễ dùng trong code.

### 6. MK_MainLogic.h - Header cho logic

**Code chính**:

```cpp
#pragma once
#include "MK_Message.h"
void taskMainLogic(void *pvParameters);
```

**Giải thích**: Chỉ khai báo taskMainLogic. Logic phức tạp trong .cpp.

### 7. MK_MainLogic.cpp - Logic chính (State Machine)

**Code chính** (đã liệt kê ở trên).
**Giải thích cặn kẽ**:

- Struct SystemContext: Lưu state, password, attempts, timeouts, phone lists.
- Enum SystemState: 4 states.
- Functions:
  - `normalizePhoneNumber()`: Lọc String, chỉ giữ số/+.
  - `addAllowedNumber()`: Thêm số vào comma-separated list, tránh duplicate.
  - `isAllowedNumber()`: Kiểm tra số trong list.
  - `handlePasswordInput()`: Xử lý key: '*' xóa, '#' xác thực (đúng: STATE2, sai: tăng attempt, khóa nếu >=3), else thêm.
  - `handleState1()`: Hiển thị "ENTER PASSWORD" sau delay.
  - `handleState2()`: Kiểm tra authTimeout, reset nếu hết.
  - `handleState3()`: Hiển thị access granted, reset sau 5s.
  - `handleState4()`: Hiển thị countdown, unlock sau 30s.
  - `taskMainLogic()`: Vòng lặp: Nhận message từ mainQueue (timeout 100ms), xử lý switch(type), update state mỗi 500ms, delay 50ms.
- **Logic**: State machine điều phối. Xử lý events (keypad, phone), timeouts, security (attempts, allowed numbers).
- **Vai trò**: Bộ não, quản lý flow và bảo mật.

### 8. MK_Keypad.h - Header cho keypad

**Code chính**:

```cpp
#pragma once
#include <Keypad.h>
#define PIN_ROW_1 16  // ... defines cho pins
#define KEYPAD_ROWS 4
#define KEYPAD_COLS 4
void keypad_init();
void taskREAD_KEYPAD(void* pvParameters);
extern volatile char key;
```

**Giải thích**: Defines pins cho 4x4 keypad. Extern volatile key cho ISR.

### 9. MK_Keypad.cpp - Implement keypad

**Code chính**:

```cpp
#include "MK_Keypad.h"
volatile char key = '0';
char hexaKeys[4][4] = {{'1','2','3','A'}, ...};
byte rowPins[4] = {16,17,18,19};
byte colPins[4] = {25,26,32,33};
Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, 4, 4);

void keypad_init() {
  xTaskCreate(taskREAD_KEYPAD, "READ_KEYPAD", 4096, NULL, 1, NULL);
}

void taskREAD_KEYPAD(void* pvParameters) {
  while(1) {
    char k = customKeypad.getKey();
    if(customKeypad.getState() == PRESSED) {
      if (k != NO_KEY) key = k;
    }
    vTaskDelay(pdMS_TO_TICKS(50));
  }
}
```

**Giải thích cặn kẽ**:

- Khởi tạo Keypad object với map keys và pins.
- `keypad_init()`: Tạo task taskREAD_KEYPAD (stack 4096, priority 1).
- `taskREAD_KEYPAD()`: Vòng lặp getKey(), set volatile key nếu pressed, delay 50ms để debounce.
- **Logic**: Polling keypad, set flag cho handler.
- **Vai trò**: Đọc input từ user.

### 10. MK_KeypadHandler.h - Header

**Code**: Chỉ khai báo taskKeypadHandler.

### 11. MK_KeypadHandler.cpp - Handler keypad

**Code chính** (đã liệt kê).
**Giải thích cặn kẽ**:

- `taskKeypadHandler()`: Kiểm tra volatile key, nếu != '0' gửi MSG_KEYPAD_PRESSED đến mainQueue, reset key, delay 50ms.
- **Logic**: Bridge giữa ISR-like keypad và logic.
- **Vai trò**: Truyền keypad events đến MainLogic.

### 12. MK_LCD.h - Header cho LCD

**Code chính**:

```cpp
#pragma once
#include <LiquidCrystal_I2C.h>
#define LCD_ADDRESS 0x27
#define LCD_ROWS 16
#define LCD_COLS 2
extern LiquidCrystal_I2C lcd;
void Module_LCD_init();
void taskMonitor_LCD(void* pvParameters);
```

**Giải thích**: Defines cho I2C LCD 16x2. Extern lcd object.

### 13. MK_LCD.cpp - Implement LCD

**Code chính**:

```cpp
#include "MK_LCD.h"
LiquidCrystal_I2C lcd(0x27, 16, 2);

void Module_LCD_init() {
  lcd.init();
  lcd.backlight();
#ifdef KEYPAD_DEBUG
  xTaskCreate(taskMonitor_LCD, "LCD_TASK", 4096, NULL, 1, NULL);
#endif
}

#ifdef KEYPAD_DEBUG
void taskMonitor_LCD(void* pvParameters) {
  uint8_t i=0, j=0;
  while(1) {
    lcd.setCursor(i, j);
    if(i >= 16) { i=0; j++; }
    if(j >= 2) j=0;
    if(key) {
      if(key != '\0') {
        lcd.print(key);
        Serial.print("Key: "); Serial.println(key);
      }
      key = 0;
    }
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}
#endif
```

**Giải thích cặn kẽ**:

- Khởi tạo lcd object.
- `Module_LCD_init()`: init() và backlight(). Nếu debug, tạo task monitor.
- `taskMonitor_LCD()`: Debug, in key lên LCD và Serial, scroll cursor.
- **Logic**: Setup LCD hardware.
- **Vai trò**: Khởi tạo display.

### 14. MK_LCDHandler.h - Header

**Code**: Chỉ khai báo taskLCDHandler.

### 15. MK_LCDHandler.cpp - Handler LCD

**Code chính** (đã liệt kê ở trên).
**Giải thích cặn kẽ**:

- Static variables: line1, line2, passwordDisplay, lastDisplayTime, lastDisplayRaw.
- `displayMessage()`: Parse msg.data ("Line1|Line2"), truncate to 16 chars, pad spaces, write to LCD with debounce (300ms).
- `taskLCDHandler()`: Vòng lặp nhận message từ displayQueue (no timeout), gọi displayMessage().
- **Logic**: Update LCD dựa trên messages, debounce để tránh flicker.
- **Vai trò**: Hiển thị UI cho user.

### 16. MK_SIM.h - Header cho SIM

**Code chính**:

```cpp
#pragma once
#define SIM_SERIAL_RX_PIN 13
#define SIM_SERIAL_TX_PIN 27
#define SIM_SERIAL Serial1
#define SIM_BAUDRATE 115200
#define SIM_SERIAL_CONFIG SERIAL_8N1
void Module_SIM_Init();
String readIncomingNumber();
void IRAM_ATTR simCallback();
extern String incomingNumber;
extern bool flagSIM;
```

**Giải thích**: Defines cho UART SIM7600. Externs cho incoming data.

### 17. MK_SIM.cpp - Implement SIM

**Code chính**:

```cpp
#include "MK_SIM.h"
String simBuffer = "";
String incomingNumber = "";
bool flagSIM = false;

void Module_SIM_Init() {
  SIM_SERIAL.begin(SIM_BAUDRATE, SIM_SERIAL_CONFIG, SIM_SERIAL_RX_PIN, SIM_SERIAL_TX_PIN);
  SIM_SERIAL.onReceive(simCallback);
  delay(10000);
  SIM_SERIAL.println("AT");
}

String readIncomingNumber() {
  String line = SIM_SERIAL.readStringUntil('\n');
  if (line.indexOf("+CLCC:") != -1) {
    int firstQuote = line.indexOf('"');
    int secondQuote = line.indexOf('"', firstQuote + 1);
    if (firstQuote != -1 && secondQuote != -1) {
      return line.substring(firstQuote + 1, secondQuote);
    }
  }
  return "";
}

void simCallback() {
  while (SIM_SERIAL.available()) {
    incomingNumber = readIncomingNumber();
    if (incomingNumber.length() > 0) {
      flagSIM = true;
    }
  }
}
```

**Giải thích cặn kẽ**:

- `Module_SIM_Init()`: Begin Serial1 với pins/baud, set callback, delay 10s (warmup), gửi "AT" test.
- `readIncomingNumber()`: Đọc line, parse "+CLCC:" để extract số trong quotes.
- `simCallback()`: ISR khi UART receive, gọi readIncomingNumber(), set flag nếu có số.
- **Logic**: UART communication với SIM7600, parse AT responses.
- **Vai trò**: Phát hiện cuộc gọi đến.

### 18. MK_SIMHandler.h - Header

**Code**: Chỉ khai báo taskSIMHandler.

### 19. MK_SIMHandler.cpp - Handler SIM

**Code chính** (đã liệt kê).
**Giải thích cặn kẽ**:

- `taskSIMHandler()`: Kiểm tra flagSIM, nếu true gửi MSG_PHONE_INCOMING với incomingNumber đến mainQueue, reset flag, delay 100ms.
- **Logic**: Bridge giữa callback và logic.
- **Vai trò**: Truyền phone events đến MainLogic.

### Tổng kết logic hoạt động toàn hệ thống

1. **Init**: main.ino → MK_app → hardware init + tasks + queues.
2. **Input**: Keypad polling → Handler → Message → MainLogic.
3. **Phone**: UART callback → Handler → Message → MainLogic.
4. **Processing**: MainLogic state machine xử lý, gửi display messages.
5. **Output**: LCD Handler update display.
6. **Security**: Password check, phone validation, timeouts, locks.
7. **RTOS**: Tasks chạy song song, queues đồng bộ, non-blocking delays.

Hệ thống đảm bảo 2FA bảo mật, responsive UI, và modularity.
