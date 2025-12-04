#ifndef MK_SIM_A7600C_H
#define MK_SIM_A7600C_H

#include <Arduino.h>

class SIM7600 {
public:
    SIM7600(HardwareSerial &serial, uint32_t baud = 115200);

    bool begin();
    String sendAT(const String &cmd, uint16_t timeout = 2000);

    // --- Nhóm cơ bản ---
    String getInfo();              // ATI
    String getIMEI();              // AT+GSN
    int getSignalQuality();        // AT+CSQ
    bool reset();                  // AT+CRESET

    // --- SMS ---
    bool sendSMS(const String &number, const String &text);
    String readSMS(uint8_t index);
    bool deleteSMS(uint8_t index);

    // --- Call ---
    bool makeCall(const String &number);
    bool hangUp();

    // --- Network ---
    bool attachGPRS();
    bool detachGPRS();
    String getIP();

    // --- GPS ---
    bool gpsPower(bool on);
    String gpsInfo();

    // --- HTTP ---
    bool httpGet(const String &url, String &response);

    // --- FTP ---
    bool ftpGet(const String &server, const String &user, const String &pass, const String &filename);

    // --- File System ---
    bool fsWrite(const String &filename, const String &data);
    String fsRead(const String &filename);

private:
    HardwareSerial *serialPort;
    uint32_t baudRate;
    String readResponse(uint16_t timeout);
};

#endif
