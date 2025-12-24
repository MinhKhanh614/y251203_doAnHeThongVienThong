#pragma once
#include <Arduino.h>
#include <queue>

// Message types
enum MessageType
{
    MSG_KEYPAD_PRESSED,   // Phím được nhấn
    MSG_PASSWORD_INPUT,   // Input password
    MSG_PASSWORD_VERIFY,  // Xác thực password
    MSG_PASSWORD_CORRECT, // Password đúng
    MSG_PASSWORD_WRONG,   // Password sai
    MSG_SYSTEM_LOCKED,    // Hệ thống bị khóa
    MSG_PHONE_INCOMING,   // Cuộc gọi đến
    MSG_PHONE_AUTH_OK,    // Xác thực qua điện thoại thành công
    MSG_AUTH_TIMEOUT,     // Timeout xác thực
    MSG_STATE_CHANGED,    // Trạng thái thay đổi
    MSG_DISPLAY_UPDATE,   // Cập nhật LCD
    MSG_ACCESS_GRANTED,   // Truy cập được cấp
    MSG_SYSTEM_RESET      // Reset hệ thống
};

// Message structure
struct Message
{
    MessageType type;
    String data;
    unsigned long timestamp;

    Message() : type(MSG_DISPLAY_UPDATE), data(""), timestamp(millis()) {}
    Message(MessageType t) : type(t), data(""), timestamp(millis()) {}
    Message(MessageType t, String d) : type(t), data(d), timestamp(millis()) {}
};

// Queue declarations (khởi tạo trong MK_Message.cpp)
// Được sử dụng để giao tiếp giữa các tasks
extern QueueHandle_t keypadQueue;
// 
extern QueueHandle_t mainQueue;
//
extern QueueHandle_t displayQueue;


// Helper functions
void sendMessage(QueueHandle_t queue, const Message &msg);
bool receiveMessage(QueueHandle_t queue, Message &msg, int timeoutMs = 0);
