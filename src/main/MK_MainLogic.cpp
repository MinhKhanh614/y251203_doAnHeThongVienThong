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
    unsigned long wrongPasswordDisplayTime; // để delay trước khi quay về ENTER PASSWORD
    String deviceNumber;                    // số của thiết bị (số mà người khác sẽ gọi đến)
    String allowedNumber;                   // comma-separated allowed phone numbers (callers)
    const int MAX_ATTEMPTS = 3;
    const int LOCK_TIME = 30000;                  // 30 giây
    const int AUTH_TIMEOUT = 30000;               // 30 giây
    const int WRONG_PASSWORD_DISPLAY_TIME = 1500; // 1.5 giây
};

SystemContext sysCtx = {
    STATE1,
    "1234567",
    "",
    0,
    0,
    0,
    0,
    "",
    "",
    3,
    30000,
    30000,
    1500};

// --- Allowed numbers list helpers (stored as comma-separated string in sysCtx.allowedNumber) ---
String normalizePhoneNumber(const String &in);
bool isAllowedNumber(const String &num); // forward declaration

void addAllowedNumber(const String &num)
{
    String norm = normalizePhoneNumber(num);
    if (norm.length() == 0)
        return;

    // avoid duplicates
    if (isAllowedNumber(norm))
        return;

    if (sysCtx.allowedNumber.length() == 0)
        sysCtx.allowedNumber = norm;
    else
        sysCtx.allowedNumber += "," + norm;
}

bool isAllowedNumber(const String &num)
{
    String norm = normalizePhoneNumber(num);
    if (norm.length() == 0 || sysCtx.allowedNumber.length() == 0)
        return false;

    int start = 0;
    while (start < sysCtx.allowedNumber.length())
    {
        int comma = sysCtx.allowedNumber.indexOf(',', start);
        String token;
        if (comma == -1)
        {
            token = sysCtx.allowedNumber.substring(start);
            start = sysCtx.allowedNumber.length();
        }
        else
        {
            token = sysCtx.allowedNumber.substring(start, comma);
            start = comma + 1;
        }
        if (token == norm)
            return true;
    }
    return false;
}

// Helper: normalize phone numbers for comparison
String normalizePhoneNumber(const String &in)
{
    String s = "";
    for (size_t i = 0; i < in.length(); ++i)
    {
        char c = in.charAt(i);
        if ((c >= '0' && c <= '9') || c == '+')
            s += c;
    }
    // convert +84xxxx -> 0xxxx
    if (s.startsWith("+84"))
    {
        return String('0') + s.substring(3);
    }
    if (s.startsWith("84") && s.length() > 2)
    {
        return String('0') + s.substring(2);
    }
    return s;
}

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

            // Set device number (the number people should call) and add allowed caller(s)
            sysCtx.deviceNumber = normalizePhoneNumber("0812373101");
            addAllowedNumber("0983305910"); // <-- đăng ký số caller để đối chiếu
            addAllowedNumber("0378468305"); // <-- đăng ký số caller để đối chiếu
            addAllowedNumber("0899715935"); // <-- đăng ký số caller để đối chiếu

            sysCtx.currentState = STATE2;
            sysCtx.authTimeout = millis() + sysCtx.AUTH_TIMEOUT;
            sysCtx.passwordInput = "";
            sysCtx.attemptCount = 0;
            // Hiển thị màn hình chờ xác thực qua điện thoại: hiển thị số thiết bị để người gọi quay số
            String displayNum = sysCtx.deviceNumber;
            if (displayNum.length() == 0)
            {
                // fallback: nếu chưa có deviceNumber thì hiển thị first allowed caller (không lý tưởng)
                displayNum = sysCtx.allowedNumber;
                int commaPos = displayNum.indexOf(',');
                if (commaPos != -1)
                    displayNum = displayNum.substring(0, commaPos);
            }
            // Truncate displayNum for 16x2 LCD (max 10 chars for "PHONE AUTH")
            if (displayNum.length() > 10)
                displayNum = displayNum.substring(0, 10);
            String line1Str = "PHONE AUTH";
            String line2Str = "CALL: " + displayNum;
            String msgData = line1Str + "|" + line2Str;
            Serial.println("[MAIN] Display message: '" + msgData + "'");
            Serial.println("[MAIN] Line1: '" + line1Str + "' (len=" + String(line1Str.length()) + ")");
            Serial.println("[MAIN] Line2: '" + line2Str + "' (len=" + String(line2Str.length()) + ")");
            Message phoneAuthMsg(MSG_DISPLAY_UPDATE, msgData);
            sendMessage(displayQueue, phoneAuthMsg);
            Serial.println("[MAIN] Password OK. Device: " + String(sysCtx.deviceNumber) + ", Allowed: " + sysCtx.allowedNumber);
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
                sysCtx.wrongPasswordDisplayTime = millis(); // set time for delay

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
    // Nếu vừa hiển thị PASSWORD_WRONG, đợi trước khi quay về ENTER PASSWORD
    if (sysCtx.wrongPasswordDisplayTime > 0 &&
        millis() - sysCtx.wrongPasswordDisplayTime > sysCtx.WRONG_PASSWORD_DISPLAY_TIME)
    {
        // Đã chờ đủ, quay về ENTER PASSWORD
        Message pwMsg(MSG_DISPLAY_UPDATE, "ENTER PASSWORD|");
        sendMessage(displayQueue, pwMsg);
        sysCtx.wrongPasswordDisplayTime = 0; // reset flag
    }
}

void handleState2()
{
    // Xác thực phone
    static unsigned long authTimeoutDisplayTime = 0;
    static bool authTimeoutDisplayed = false;

    // Kiểm tra timeout
    if (millis() > sysCtx.authTimeout && !authTimeoutDisplayed)
    {
        Message displayMsg(MSG_AUTH_TIMEOUT);
        sendMessage(displayQueue, displayMsg);
        authTimeoutDisplayTime = millis();
        authTimeoutDisplayed = true;
    }

    // Sau delay, quay về STATE1 và hiển thị ENTER PASSWORD
    if (authTimeoutDisplayed && (millis() - authTimeoutDisplayTime > 2000))
    {
        sysCtx.currentState = STATE1;
        sysCtx.passwordInput = "";
        sysCtx.attemptCount = 0;
        authTimeoutDisplayed = false;

        // Hiển thị lại prompt nhập password
        Message pwMsg(MSG_DISPLAY_UPDATE, "ENTER PASSWORD|");
        sendMessage(displayQueue, pwMsg);
    }
}

void handleState3()
{
    // Truy cập được cấp - có thể thêm chức năng ở đây
    // Hiển thị trạng thái hệ thống
    static unsigned long accessGrantedTime = 0;
    static bool accessGrantedDisplayed = false;

    if (!accessGrantedDisplayed)
    {
        Message displayMsg(MSG_ACCESS_GRANTED);
        sendMessage(displayQueue, displayMsg);
        accessGrantedTime = millis();
        accessGrantedDisplayed = true;
    }

    // Quay lại state 1 sau một thời gian (ví dụ: 5 giây)
    if (millis() - accessGrantedTime > 5000)
    {
        sysCtx.currentState = STATE1;
        sysCtx.passwordInput = "";
        sysCtx.attemptCount = 0;
        accessGrantedDisplayed = false;

        // Hiển thị lại prompt nhập password
        Message pwMsg(MSG_DISPLAY_UPDATE, "ENTER PASSWORD|");
        sendMessage(displayQueue, pwMsg);
    }
}

void handleState4()
{
    // Hệ thống bị khóa
    unsigned long now = millis();

    // Kiểm tra xem timeout đã đạt chưa (so sánh đơn giản để tránh tràn)
    if (now < sysCtx.lockTimeout)
    {
        // Cập nhật thời gian còn lại
        unsigned long remainingTime = sysCtx.lockTimeout - now;
        unsigned long secondsLeft = remainingTime / 1000;
        // Đảm bảo không hiển thị số âm (nếu vượt quá sẽ hiển thị 0)
        Message displayMsg(MSG_SYSTEM_LOCKED, String(secondsLeft > 0 ? secondsLeft : 0));
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
        // Nhận message từ keypad/phone (không block - timeout 100ms để đủ thời gian nhận)
        if (receiveMessage(mainQueue, msg, 100))
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
                        sendMessage(displayQueue, displayMsg);

                        sysCtx.currentState = STATE3;
                        // reset auth timeout
                        sysCtx.authTimeout = 0;
                    }
                    else
                    {
                        // Caller không khớp; có thể log hoặc thông báo
                        Serial.println("[MAIN] AUTH FAILED - Caller not in allowed list");
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
