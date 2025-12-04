#include "MK_MainLogic.h"

// State machine
enum SystemState
{
    STATE1 = 0, // Nhập password
    STATE2 = 1, // Xác thực phone
    STATE3 = 2, // Truy cập được cấp
    STATE4 = 3  // Hệ thống bị khóa
};

// System variables
struct SystemContext
{
    SystemState currentState;
    String password;
    String passwordInput;
    int attemptCount;
    unsigned long lockTimeout;
    unsigned long authTimeout;
    String allowedNumber; // số điện thoại được phép xác thực
    const int MAX_ATTEMPTS = 3;
    const int LOCK_TIME = 30000;    // 30 giây
    const int AUTH_TIMEOUT = 30000; // 30 giây
};

SystemContext sysCtx = {
    STATE1,
    "1234567",
    "",
    0,
    0,
    0,
    "",
    3,
    30000,
    30000};

void handlePasswordInput(char key)
{
    if (key == '*') // Xóa ký tự cuối
    {
        if (sysCtx.passwordInput.length() > 0)
            sysCtx.passwordInput.remove(sysCtx.passwordInput.length() - 1);
    }
    else if (key == '#') // Xác nhận password
    {
        if (sysCtx.passwordInput == sysCtx.password)
        {
            // Password đúng - gửi message
            Message displayMsg(MSG_PASSWORD_CORRECT);
            sendMessage(displayQueue, displayMsg);

            // set allowed phone number for auth (the configured number)
            sysCtx.allowedNumber = "0812373101"; // <-- khai báo số điện thoại mong muốn

            sysCtx.currentState = STATE2;
            sysCtx.authTimeout = millis() + sysCtx.AUTH_TIMEOUT;
            sysCtx.passwordInput = "";
            sysCtx.attemptCount = 0;
            // Hiển thị màn hình chờ xác thực qua điện thoại kèm số
            Message phoneAuthMsg(MSG_DISPLAY_UPDATE, String("PHONE AUTH|CALL: ") + sysCtx.allowedNumber);
            sendMessage(displayQueue, phoneAuthMsg);
        }
        else
        {
            // Password sai
            sysCtx.attemptCount = sysCtx.attemptCount + 1;

            if (sysCtx.attemptCount >= sysCtx.MAX_ATTEMPTS)
            {
                // Khóa hệ thống
                Message displayMsg(MSG_SYSTEM_LOCKED, "30");
                sendMessage(displayQueue, displayMsg);

                sysCtx.currentState = STATE4;
                sysCtx.lockTimeout = millis() + sysCtx.LOCK_TIME;
            }
            else
            {
                // Hiển thị lỗi
                int triesLeft = sysCtx.MAX_ATTEMPTS - sysCtx.attemptCount;
                Message displayMsg(MSG_PASSWORD_WRONG, String(triesLeft));
                sendMessage(displayQueue, displayMsg);

                sysCtx.passwordInput = "";
            }
        }
    }
    else // Thêm ký tự vào password
    {
        sysCtx.passwordInput += key;
    }

    // Gửi message để cập nhật hiển thị
    Message pwMsg(MSG_PASSWORD_INPUT, sysCtx.passwordInput);
    sendMessage(displayQueue, pwMsg);
}

void handleState1()
{
    // Nhập password - xử lý từ keypad input
    // Không có hành động khác cần thiết, chỉ chờ keypad
}

void handleState2()
{
    // Xác thực phone

    // Kiểm tra timeout
    if (millis() > sysCtx.authTimeout)
    {
        Message displayMsg(MSG_AUTH_TIMEOUT);
        sendMessage(displayQueue, displayMsg);

        sysCtx.currentState = STATE1;
        sysCtx.passwordInput = "";
        sysCtx.attemptCount = 0;

        // Hiển thị lại prompt nhập password
        Message pwMsg(MSG_DISPLAY_UPDATE, "ENTER PASSWORD|");
        sendMessage(displayQueue, pwMsg);
    }
}

void handleState3()
{
    // Truy cập được cấp - có thể thêm chức năng ở đây
    // Hiển thị trạng thái hệ thống
    Message displayMsg(MSG_ACCESS_GRANTED);
    sendMessage(displayQueue, displayMsg);

    // TODO: Thực hiện các chức năng chính ở đây

    // Quay lại state 1 sau một thời gian (ví dụ: 30 giây)
    delay(30000);
    sysCtx.currentState = STATE1;
    sysCtx.passwordInput = "";
    sysCtx.attemptCount = 0;
}

void handleState4()
{
    // Hệ thống bị khóa
    unsigned long remainingTime = sysCtx.lockTimeout - millis();

    if (remainingTime > 0)
    {
        // Cập nhật thời gian còn lại
        unsigned long secondsLeft = remainingTime / 1000;
        Message displayMsg(MSG_SYSTEM_LOCKED, String(secondsLeft));
        sendMessage(displayQueue, displayMsg);
    }
    else
    {
        // Reset hệ thống
        sysCtx.currentState = STATE1;
        sysCtx.passwordInput = "";
        sysCtx.attemptCount = 0;
        sysCtx.lockTimeout = 0;

        Message displayMsg(MSG_DISPLAY_UPDATE, "ENTER PASSWORD|");
        sendMessage(displayQueue, displayMsg);
    }
}

void taskMainLogic(void *pvParameters)
{
    Message msg;
    unsigned long lastUpdateTime = millis();
    const int UPDATE_INTERVAL = 500; // Cập nhật trạng thái mỗi 500ms

    while (1)
    {
        // Nhận message từ keypad (không block)
        if (receiveMessage(mainQueue, msg, 10))
        {
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
                    if (msg.data.length() > 0 && (sysCtx.allowedNumber.length() == 0 || msg.data == sysCtx.allowedNumber))
                    {
                        Message displayMsg(MSG_PHONE_AUTH_OK, msg.data);
                        sendMessage(displayQueue, displayMsg);

                        sysCtx.currentState = STATE3;
                        // reset auth timeout
                        sysCtx.authTimeout = 0;
                    }
                    else
                    {
                        // Caller không khớp; có thể log hoặc thông báo
                        Message badCaller(MSG_PASSWORD_WRONG, "Invalid caller");
                        sendMessage(displayQueue, badCaller);
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

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}
