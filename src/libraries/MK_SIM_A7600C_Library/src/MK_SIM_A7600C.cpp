#include "MK_SIM_A7600C.h"

SIM7600::SIM7600(HardwareSerial &serial, uint32_t baud) {
    serialPort = &serial;
    baudRate = baud;
}

bool SIM7600::begin() {
    serialPort->begin(baudRate);
    delay(1000);
    String resp = sendAT("AT");
    return resp.indexOf("OK") != -1;
}

String SIM7600::sendAT(const String &cmd, uint16_t timeout) {
    // Flush buffer trước
    while (serialPort->available()) {
        serialPort->read();
    }
    delay(50);
    
    serialPort->println(cmd);
    return readResponse(timeout);
}

String SIM7600::readResponse(uint16_t timeout) {
    String resp = "";
    uint32_t start = millis();
    const uint16_t MAX_RESPONSE = 2048;  // Giới hạn kích thước
    
    while (millis() - start < timeout) {
        while (serialPort->available()) {
            char c = serialPort->read();
            if (resp.length() < MAX_RESPONSE) {
                resp += c;
            }
        }
        // Thoát sớm nếu nhận được OK hoặc ERROR
        if (resp.indexOf("OK") != -1 || resp.indexOf("ERROR") != -1) {
            break;
        }
    }
    return resp;
}

// --- Basic ---
String SIM7600::getInfo() { return sendAT("ATI"); }
String SIM7600::getIMEI() { 
    String resp = sendAT("AT+GSN");
    // Xóa dòng "AT+GSN" nếu có
    int idx = resp.indexOf("\n");
    if (idx != -1) {
        resp = resp.substring(idx + 1);
    }
    resp.trim();
    return resp;
}
int SIM7600::getSignalQuality() {
    String resp = sendAT("AT+CSQ");
    int rssi = -1;
    sscanf(resp.c_str(), "+CSQ: %d", &rssi);
    return rssi;
}
bool SIM7600::reset() { return sendAT("AT+CRESET").indexOf("OK") != -1; }

// --- SMS ---
bool SIM7600::sendSMS(const String &number, const String &text) {
    if (sendAT("AT+CMGF=1", 2000).indexOf("OK") == -1) return false;
    delay(200);
    if (sendAT("AT+CMGS=\"" + number + "\"", 2000).indexOf(">") == -1) return false;
    delay(200);
    serialPort->print(text);
    serialPort->write(0x1A); // Ctrl+Z
    String resp = readResponse(3000);
    return resp.indexOf("OK") != -1;
}

// --- Call ---
bool SIM7600::makeCall(const String &number) {
    return sendAT("ATD" + number + ";").indexOf("OK") != -1;
}
bool SIM7600::hangUp() { return sendAT("ATH").indexOf("OK") != -1; }

// --- Network ---
bool SIM7600::attachGPRS() { return sendAT("AT+CGATT=1").indexOf("OK") != -1; }
bool SIM7600::detachGPRS() { return sendAT("AT+CGATT=0").indexOf("OK") != -1; }
String SIM7600::getIP() { return sendAT("AT+CIFSR"); }

// --- GPS ---
bool SIM7600::gpsPower(bool on) {
    return sendAT(on ? "AT+CGPS=1" : "AT+CGPS=0").indexOf("OK") != -1;
}
String SIM7600::gpsInfo() { return sendAT("AT+CGPSINFO"); }

// --- HTTP ---
bool SIM7600::httpGet(const String &url, String &response) {
    sendAT("AT+CHTTPACT=\"" + url + "\",80");
    response = readResponse(10000);
    return response.indexOf("OK") != -1;
}

// --- FTP ---
bool SIM7600::ftpGet(const String &server, const String &user, const String &pass, const String &filename) {
    sendAT("AT+CFTPSERV=\"" + server + "\"");
    sendAT("AT+CFTPUN=\"" + user + "\"");
    sendAT("AT+CFTPPW=\"" + pass + "\"");
    String resp = sendAT("AT+CFTPGETFILE=\"" + filename + "\"");
    return resp.indexOf("OK") != -1;
}

// --- File System ---
bool SIM7600::fsWrite(const String &filename, const String &data) {
    sendAT("AT+FSWRITE=\"" + filename + "\",0," + String(data.length()) + ",10");
    serialPort->print(data);
    String resp = readResponse(5000);
    return resp.indexOf("OK") != -1;
}
String SIM7600::fsRead(const String &filename) {
    return sendAT("AT+FSREAD=\"" + filename + "\",0,100");
}
