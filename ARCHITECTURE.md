# Kiến Trúc Hệ Thống Task-Based

## Tổng Quan

Dự án đã được refactor để sử dụng **Task-based architecture** với **message queue** cho trao đổi dữ liệu giữa các modules.

## Kiến Trúc

```
┌─────────────────────────────────────────────────────────────┐
│                        main.ino                             │
│                      (Entry Point)                          │
└──────────────────────┬──────────────────────────────────────┘
                       │
                       ▼
        ┌──────────────────────────────────┐
        │      app_run() in MK_app.cpp     │
        │  (Initialize Hardware & Tasks)   │
        └──────────────────────────────────┘
                       │
        ┌──────────────┼──────────────┬─────────────┐
        │              │              │             │
        ▼              ▼              ▼             ▼
   ┌─────────────┐ ┌─────────────┐ ┌──────────┐ ┌─────────────┐
   │   Keypad    │ │    LCD      │ │  Main    │ │    SIM      │
   │   Handler   │ │  Handler    │ │  Logic   │ │   Handler   │
   │   (Task)    │ │  (Task)     │ │ (Task)   │ │   (Task)    │
   └──────┬──────┘ └──────┬──────┘ └────┬─────┘ └──────┬──────┘
          │               │              │             │
          └───────────────┼──────────────┼─────────────┘
                          │
                    ┌─────▼──────┐
                    │   Message  │
                    │   Queues   │
                    └────────────┘
```

## Các Tasks

### 1. **taskKeypadHandler** (Priority: 2)

- **Nhiệm vụ**: Đọc phím từ keypad
- **Đầu vào**: Phím từ MK_Keypad ISR
- **Đầu ra**: Gửi `MSG_KEYPAD_PRESSED` vào `mainQueue`
- **Stack**: 2048 bytes

### 2. **taskLCDHandler** (Priority: 1)

- **Nhiệm vụ**: Hiển thị thông tin trên LCD
- **Đầu vào**: Nhận message từ `displayQueue`
- **Đầu ra**: Cập nhật LCD
- **Stack**: 3072 bytes

### 3. **taskMainLogic** (Priority: 2)

- **Nhiệm vụ**: Xử lý logic chính của hệ thống
- **Đầu vào**: Nhận message từ `mainQueue` (keypad, phone, etc.)
- **Đầu ra**: Gửi message tới `displayQueue` để hiển thị
- **Stack**: 4096 bytes
- **State Machine**:
  - `STATE1`: Nhập password
  - `STATE2`: Xác thực qua điện thoại
  - `STATE3`: Truy cập được cấp
  - `STATE4`: Hệ thống bị khóa

### 4. **taskSIMHandler** (Priority: 2)

- **Nhiệm vụ**: Xử lý module SIM7600
- **Đầu vào**: Incoming calls từ SIM
- **Đầu ra**: Gửi `MSG_PHONE_INCOMING` vào `mainQueue`
- **Stack**: 2048 bytes

## Message System

### Message Structure

```cpp
struct Message {
  MessageType type;       // Loại message
  String data;           // Dữ liệu (payload)
  unsigned long timestamp; // Thời gian tạo
};
```

### Message Types

| Type | Mô Tả | Dữ Liệu |
|------|-------|---------|
| `MSG_KEYPAD_PRESSED` | Phím được nhấn | Ký tự phím |
| `MSG_PASSWORD_INPUT` | Input password | Chuỗi password |
| `MSG_PASSWORD_CORRECT` | Password đúng | (none) |
| `MSG_PASSWORD_WRONG` | Password sai | Số lần còn lại |
| `MSG_SYSTEM_LOCKED` | Hệ thống bị khóa | Thời gian còn lại |
| `MSG_PHONE_INCOMING` | Cuộc gọi đến | Số điện thoại |
| `MSG_PHONE_AUTH_OK` | Xác thực thành công | Số điện thoại |
| `MSG_AUTH_TIMEOUT` | Timeout xác thực | (none) |
| `MSG_ACCESS_GRANTED` | Truy cập được cấp | (none) |
| `MSG_DISPLAY_UPDATE` | Cập nhật LCD | "Line1\|Line2" |

### Message Queues

- **mainQueue** (10 items):
  - Nhận: Keypad input, Phone incoming
  - Gửi bởi: Keypad Handler, SIM Handler
  - Sử dụng: Main Logic

- **displayQueue** (10 items):
  - Nhận: Display messages
  - Gửi bởi: Main Logic
  - Sử dụng: LCD Handler

## Flow Ví Dụ: Nhập Password

```
1. User nhấn phím '1'
   └─> Keypad Handler nhận từ ISR
       └─> Gửi MSG_KEYPAD_PRESSED('1') vào mainQueue

2. Main Logic nhận message
   └─> handlePasswordInput('1')
       └─> Thêm '1' vào passwordInput
       └─> Gửi MSG_PASSWORD_INPUT("1") vào displayQueue

3. LCD Handler nhận message
   └─> Hiển thị "*" trên LCD

4. User nhấn '#' (confirm)
   └─> Keypad Handler gửi MSG_KEYPAD_PRESSED('#')

5. Main Logic xác thực password
   └─> Nếu đúng:
       └─> Gửi MSG_PASSWORD_CORRECT
       └─> Đổi state sang STATE2
   └─> Nếu sai:
       └─> Gửi MSG_PASSWORD_WRONG(triesLeft)
```

## Ưu Điểm của Kiến Trúc Này

✅ **Modular**: Mỗi module là độc lập
✅ **Scalable**: Dễ thêm features mới
✅ **Responsive**: Task có priority riêng
✅ **Maintainable**: Código rõ ràng, dễ bảo trì
✅ **Thread-safe**: Message queue là thread-safe
✅ **Decoupled**: Modules không phụ thuộc lẫn nhau

## Các File Liên Quan

| File | Mục Đích |
|------|----------|
| `MK_Message.h/cpp` | Message queue system |
| `MK_KeypadHandler.h/cpp` | Keypad task |
| `MK_LCDHandler.h/cpp` | LCD task |
| `MK_MainLogic.h/cpp` | Main state machine |
| `MK_SIMHandler.h/cpp` | SIM module task |
| `MK_app.h/cpp` | Khởi tạo app & tasks |

## Tiếp Theo

Để extend hệ thống, bạn có thể:

1. Thêm task mới cho chức năng khác
2. Định nghĩa message type mới trong `MK_Message.h`
3. Gửi/nhận messages qua queue
4. Main Logic xử lý logic business

Ví dụ thêm task Storage:

```cpp
// MK_StorageHandler.h
void taskStorageHandler(void *pvParameters);

// MK_StorageHandler.cpp
void taskStorageHandler(void *pvParameters) {
  while(1) {
    Message msg;
    if(receiveMessage(mainQueue, msg, 100)) {
      // Xử lý storage operations
    }
  }
}
```
