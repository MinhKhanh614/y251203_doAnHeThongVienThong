#!/bin/bash
# Build script cho ESP32 project

# Kiểm tra xem Arduino CLI có được cài không
if ! command -v arduino-cli &> /dev/null; then
    echo "Arduino CLI không được tìm thấy"
    echo "Cài đặt từ: https://arduino.cc/en/software"
    exit 1
fi

# Build project
echo "Building project..."
arduino-cli compile --fqbn esp32:esp32:esp32 src/main/main.ino

if [ $? -eq 0 ]; then
    echo "✓ Build thành công!"
else
    echo "✗ Build thất bại!"
    exit 1
fi
