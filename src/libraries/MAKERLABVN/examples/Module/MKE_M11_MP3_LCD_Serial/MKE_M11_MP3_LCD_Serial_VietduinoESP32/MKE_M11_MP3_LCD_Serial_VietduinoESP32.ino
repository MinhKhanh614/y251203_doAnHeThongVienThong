#include <MKL_LiquidCrystal_I2C.h>
#include <MKL_DFRobotDFPlayerMini.h>

// Khởi tạo LCD
// LCD config
MKL_LiquidCrystal_I2C lcd(0x27, 16, 2);

// Define RX and TX pins for ESP32 Serial2
#define RX_PIN 18
#define TX_PIN 19

// Khai báo module
// Declare module
MKL_DFRobotDFPlayerMini myDFPlayer;

unsigned long timeDelay = 2000; // 2000ms

void setup()
{
  // Khởi động LCD
  // LCD start
  lcd.init();
  lcd.backlight();

  // Khởi động kết nối Serial UART ở tốc độ 115200 để truyền dữ liệu lên máy tính
  // Start the Serial UART connection at 115200 to transfer data to the computer
  Serial.begin(115200);

  // Khởi tạo module với Serial2
  // Initialize module with Serial2
  Serial2.begin(9600, SERIAL_8N1, RX_PIN, TX_PIN);
  delay(200);
  myDFPlayer.begin(Serial2, true, true);
  delay(100);
  
  myDFPlayer.setTimeOut(500);
  myDFPlayer.volume(30); // Volume range: 0-30
}

void loop()
{
  myDFPlayer.playMp3Folder(1);

  // Hiển thị giá trị của module lên máy tính
  // Show the module value on Arduino Serial Monitor
  Serial.println("Play file 1");

  // Gửi giá trị module lên LCD
  // Show the module value on LCD
  lcd.setCursor(0, 0);
  lcd.print("Play file 1    "); // Thêm khoảng trắng để xóa văn bản cũ

  // Chờ 2000ms
  // Wait 2000ms
  delay(2000);

  myDFPlayer.stop();

  // Hiển thị giá trị của module lên máy tính
  // Show the module value on Arduino Serial Monitor
  Serial.println("Stop file 1");

  // Gửi giá trị module lên LCD
  // Show the module value on LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Stop file 1    "); // Thêm khoảng trắng để xóa văn bản cũ

  // Chờ 2000ms
  // Wait 2000ms
  delay(2000);
}