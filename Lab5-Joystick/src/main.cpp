#include "Adafruit_GFX.h"     //TFT screen // Core graphics library
#include "Adafruit_ST7735.h"  // Hardware-specific library for ST7735
#include "Arduino.h"
#include "ArduinoJson.h"  //JSON packaging
#include "MPU6050.h"      //IMU Inertial Measurement Unit MPU6050
#include "RPC.h"
#include "TM1637TinyDisplay.h"  //TM1637 7-segment 4x display
#include "WiFi.h"
#include "mbed.h"
#include "rtos.h"

using namespace mbed;
using namespace rtos;
using namespace std::chrono;

// Joystick stuff
const uint8_t BUTTON_UP = D2;
const uint8_t BUTTON_RIGHT = D3;
const uint8_t BUTTON_DOWN = D4;
const uint8_t BUTTON_LEFT = D5;
const uint8_t BUTTON_E = D6;
const uint8_t BUTTON_F = D7;
const uint8_t BUTTON_K = D8;
const uint8_t PIN_ANALOG_X = A0;
const uint8_t PIN_ANALOG_Y = A1;

PinStatus ButtonUpState = HIGH;
PinStatus ButtonRightState = HIGH;
PinStatus ButtonDownState = HIGH;
PinStatus ButtonLeftState = HIGH;
PinStatus ButtonEState = HIGH;
PinStatus ButtonFState = HIGH;
PinStatus ButtonKState = HIGH;
unsigned int JoystickAnalogX = 512;
unsigned int JoystickAnalogY = 512;
PinStatus JoystickXRightState = HIGH;
PinStatus JoystickXLeftState = HIGH;
PinStatus JoystickYUpState = HIGH;
PinStatus JoystickYDownState = HIGH;
const int joystickDeadband = 50;
int8_t x = 0;
int8_t y = 0;
int8_t z = 0;
int8_t xLast = 0;
int8_t yLast = 0;
int8_t zLast = 0;

// TM1637 4-segments display
const uint8_t CLK = A2;
const uint8_t DIO = A3;
TM1637TinyDisplay display(CLK, DIO);  //,0;
bool displayDots = true;
byte minuteRunning = 0;
byte secondRunning = 0;

// inertial measurement unit
float Temp = 0;
float pitch = 0;
float roll = 0;
float yaw = 0;
float pitchLast = 0;
float rollLast = 0;
float yawLast = 0;
float xBall = 0;
float yBall = 0;
float xBallLast = 0;
float yBallLast = 0;

// laser output
PinStatus laserState = LOW;

// thin-film-transistor liquid-crystal display
const uint8_t TFT_CS = D13;
const uint8_t TFT_DC = D11;
const uint8_t TFT_MOSI = D10;
const uint8_t TFT_SCLK = D9;
const uint8_t TFT_RST = D12;
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);

// WiFi for udp, ur3 comm, telemetry, http, etc.
const char ssid[] = "patata";  // Access point
const char pass[] = "patata1234";  // Access point
int status = WL_IDLE_STATUS;     // the WiFi radio's status
IPAddress ip(192, 168, 1, 111);  // static ip is not working!
WiFiUDP Udp;
IPAddress segwayIp(192, 168, 1, 112);
unsigned int segwayPort = 8888;

// a mutex used when printing
Mutex printMutex;  // Since we're printing from multiple threads, we need a mutex

// capacitive touch button
PinStatus capacitiveValue = LOW;

// additional global variables
unsigned int counter = 0;

// Function prototypes
void ledInit(void);
void ledUpdate(void);
void wifiInitAccessPoint(void);
void wifiInitWPA(void);
void tftInit(void);
void tftUpdate(void);
void motorInit(void);
void motorON(void);
void motorOFF(void);
void laserInit(void);
void laserON(void);
void laserOFF(void);
void capacitiveInit(void);
void capacitiveUpdate(void);
void imuInit(void);
void imuGetData(void);
void gpsInit(void);
void gpsGetData(void);
void joystickInit(void);
void joystickGetData(void);
void displayInit(void);
void displayUpdate(void);
void supervisionUpdate(void);
void sendCommand(uint8_t command);

void setup() {
    Serial.begin(115200);
    Serial.println("Init...");

    ledInit();
    wifiInitAccessPoint();  // wifiInitWPA();
    tftInit();
    motorInit();
    laserInit();
    capacitiveInit();
    imuInit();
    gpsInit();
    joystickInit();
    displayInit();
}

void loop() {
    ledUpdate();
    if (laserState == LOW) {
        laserON();
    } else {
        laserOFF();
    }
    capacitiveUpdate();
    if (capacitiveValue == HIGH) {
        motorON();
    } else {
        motorOFF();
    }
    imuGetData();
    gpsGetData();
    joystickGetData();
    displayUpdate();
    tftUpdate();
    supervisionUpdate();

    const uint8_t data = 0xF1;
    sendCommand(data);

    delay(1000);
}

void sendCommand(uint8_t command) {
    const int validClientSettings = Udp.beginPacket(segwayIp, segwayPort);
    Serial.print("Sending command to ");
    Serial.print(segwayIp);
    Serial.print(":");
    Serial.print(segwayPort);
    Serial.print(" with command: ");
    Serial.println(command, HEX);
    if (!validClientSettings) {
        Serial.println("Error: UDP beginPacket failed");
        return;
    }

    const int writtenDataLength = Udp.write(command);
    if (writtenDataLength == 0) {
        Serial.println("Error: UDP write failed");
        return;
    }

    const int dataSentSuccessfully = Udp.endPacket();
    if (dataSentSuccessfully == 0) {
        Serial.println("Error: UDP endPacket failed");
    }
    else {
        Serial.println("Data sent successfully");
    }
}

void ledInit(void) {
    pinMode(LEDR, OUTPUT);
    pinMode(LEDG, OUTPUT);
    pinMode(LEDB, OUTPUT);
}

void ledUpdate(void) {
    digitalWrite(LEDR, random(0, 2));  // random(min,max)//min is inclusive, max is exclusive
    digitalWrite(LEDG, random(0, 2));
    digitalWrite(LEDB, random(0, 2));
}

void wifiInitAccessPoint(void) {
    // check for the WiFi module:
    if (WiFi.status() == WL_NO_MODULE) {
        Serial.println("Communication with WiFi module failed!");
    }
    WiFi.config(ip);
    // print the network name (SSID);
    Serial.print("Creating access point named: ");
    Serial.println(ssid);
    // Create open network. Change this line if you want to create an WEP network:
    int status = WL_IDLE_STATUS;
    status = WiFi.beginAP(ssid, pass);  // wifi.beginAP(const char* ssid, const char* passphrase, uint8_t channel = DEFAULT_AP_CHANNEL);
    if (status != WL_AP_LISTENING) {
        Serial.println("Creating access point failed");
    }
    // print the SSID of the network you're attached to:
    Serial.print("SSID: ");
    Serial.println(WiFi.SSID());

    // print your board's IP address:
    IPAddress ip = WiFi.localIP();
    Serial.print("IP Address: ");
    Serial.println(ip);

    // print the received signal strength:
    long rssi = WiFi.RSSI();
    Serial.print("signal strength (RSSI):");
    Serial.print(rssi);
    Serial.println(" dBm");
    // print the SSID of the network you're attached to:
    Serial.print("SSID: ");
    Serial.println(WiFi.SSID());

    // print the encryption type:
    byte encryption = WiFi.encryptionType();
    Serial.print("Encryption Type:");
    Serial.println(encryption, HEX);
    Serial.println();
    // print your board's IP address:
    ip = WiFi.localIP();
    Serial.print("IP Address: ");
    Serial.println(ip);
    Serial.println(ip);

    // print your MAC address:
    byte mac[6];
    WiFi.macAddress(mac);
    Serial.print("MAC address: ");
    for (int i = 5; i >= 0; i--) {
        if (mac[i] < 16) {
            Serial.print("0");
        }
        Serial.print(mac[i], HEX);
        if (i > 0) {
            Serial.print(":");
        }
    }
    Serial.println();
}

void wifiInitWPA(void) {
    // check for the WiFi module:
    if (WiFi.status() == WL_NO_MODULE) {
        Serial.println("Communication with WiFi module failed!");
    }
    WiFi.config(ip);
    // print the network name (SSID);
    Serial.print("Creating access point named: ");
    Serial.println(ssid);
    // Create open network. Change this line if you want to create an WEP network:
    int status = WL_IDLE_STATUS;
    status = WiFi.begin(ssid, pass);
    if (status != WL_AP_LISTENING) {
        Serial.println("Creating access point failed");
    }
    // print the SSID of the network you're attached to:
    Serial.print("SSID: ");
    Serial.println(WiFi.SSID());

    // print your board's IP address:
    IPAddress ip = WiFi.localIP();
    Serial.print("IP Address: ");
    Serial.println(ip);

    // print the received signal strength:
    long rssi = WiFi.RSSI();
    Serial.print("signal strength (RSSI):");
    Serial.print(rssi);
    Serial.println(" dBm");
    // print the SSID of the network you're attached to:
    Serial.print("SSID: ");
    Serial.println(WiFi.SSID());

    // print the encryption type:
    byte encryption = WiFi.encryptionType();
    Serial.print("Encryption Type:");
    Serial.println(encryption, HEX);
    Serial.println();
    // print your board's IP address:
    ip = WiFi.localIP();
    Serial.print("IP Address: ");
    Serial.println(ip);
    Serial.println(ip);

    // print your MAC address:
    byte mac[6];
    WiFi.macAddress(mac);
    Serial.print("MAC address: ");
    for (int i = 5; i >= 0; i--) {
        if (mac[i] < 16) {
            Serial.print("0");
        }
        Serial.print(mac[i], HEX);
        if (i > 0) {
            Serial.print(":");
        }
    }
    Serial.println();
}

void tftInit(void) {
    tft.initR(INITR_BLACKTAB);  // Init ST7735S chip 128x160 black tab
    // tft.setFont(&FreeSerif9pt7b);
    tft.fillScreen(0xffff);
    // tft.fillScreen(0x0000);
    tft.setTextWrap(false);
    tft.setTextSize(1);
    tft.setTextColor(0x0000, 0xffff);
    tft.cp437(true);  // Use correct CP437 character codes

    tft.fillScreen(0xffff);
    // tft.fillScreen(0x0000);
    tft.setCursor(20, 80);
    // tft.setTextColor(0x681f,0xffff);
    // tft.print("Initializing...");
    tft.setTextColor(0x0000, 0xffff);
    tft.setCursor(10, 3);
    tft.print(ip);
    tft.fillCircle(5, 6, 2, 0x07fc);
    tft.drawCircle(5, 6, 3, 0x195f);

    tft.drawCircle(115, 140, 2, 0xd0a0);                  // Up-button
    tft.drawCircle(120, 145, 2, 0xd0a0);                  // Right-button
    tft.drawCircle(115, 150, 2, 0xd0a0);                  // Down-button
    tft.drawCircle(110, 145, 2, 0xd0a0);                  // Left-button
    tft.drawCircle(104, 150, 2, 0xd0a0);                  // E-button
    tft.drawCircle(98, 150, 2, 0xd0a0);                   // F-button
    tft.drawCircle(87, 145, 2, 0xd0a0);                   // K-button
    tft.drawTriangle(87, 136, 84, 141, 90, 141, 0xd0a0);  // Joy +y
    tft.drawTriangle(91, 142, 91, 148, 96, 145, 0xd0a0);  // Joy +x
    tft.drawTriangle(87, 154, 84, 149, 90, 149, 0xd0a0);  // Joy -y
    tft.drawTriangle(78, 145, 83, 148, 83, 142, 0xd0a0);  // Joy -x
}

void tftUpdate(void) {
    tft.setTextColor(0x0000, 0xffff);
    tft.setCursor(5, 140);
    tft.print("lat=");
    tft.print(lat, 6);
    // tft.write(0xF8);

    tft.setTextColor(0x0000, 0xffff);
    tft.setCursor(5, 150);
    tft.print("lon=");
    tft.print(lon, 6);
    // tft.write(0xF8);

    if (ButtonUpState == LOW) {
        tft.fillCircle(115, 140, 2, 0xd0a0);  // Up-button
    } else {
        tft.fillCircle(115, 140, 2, 0xffff);  // Up-button
        tft.drawCircle(115, 140, 2, 0xd0a0);  // Up-button1
    }
    if (ButtonRightState == LOW) {
        tft.fillCircle(120, 145, 2, 0xd0a0);
    } else {
        tft.fillCircle(120, 145, 2, 0xffff);
        tft.drawCircle(120, 145, 2, 0xd0a0);
    }
    if (ButtonDownState == LOW) {
        tft.fillCircle(115, 150, 2, 0xd0a0);
    } else {
        tft.fillCircle(115, 150, 2, 0xffff);
        tft.drawCircle(115, 150, 2, 0xd0a0);
    }
    if (ButtonLeftState == LOW) {
        tft.fillCircle(110, 145, 2, 0xd0a0);
    } else {
        tft.fillCircle(110, 145, 2, 0xffff);
        tft.drawCircle(110, 145, 2, 0xd0a0);
    }
    if (ButtonEState == LOW) {
        tft.fillCircle(104, 150, 2, 0xd0a0);
    } else {
        tft.fillCircle(104, 150, 2, 0xffff);
        tft.drawCircle(104, 150, 2, 0xd0a0);
    }
    if (ButtonFState == LOW) {
        tft.fillCircle(98, 150, 2, 0xd0a0);
    } else {
        tft.fillCircle(98, 150, 2, 0xffff);
        tft.drawCircle(98, 150, 2, 0xd0a0);
    }
    if (ButtonKState == LOW) {
        tft.fillCircle(87, 145, 2, 0xd0a0);
    } else {
        tft.fillCircle(87, 145, 2, 0xffff);
        tft.drawCircle(87, 145, 2, 0xd0a0);
    }
    if (JoystickXRightState == HIGH) {
        tft.fillTriangle(91, 142, 91, 148, 96, 145, 0xd0a0);
    } else {
        tft.fillTriangle(91, 142, 91, 148, 96, 145, 0xffff);
        tft.drawTriangle(91, 142, 91, 148, 96, 145, 0xd0a0);
    }
    if (JoystickXLeftState == HIGH) {
        tft.fillTriangle(78, 145, 83, 148, 83, 142, 0xd0a0);
    } else {
        tft.fillTriangle(78, 145, 83, 148, 83, 142, 0xffff);
        tft.drawTriangle(78, 145, 83, 148, 83, 142, 0xd0a0);
    }
    if (JoystickYUpState == HIGH) {
        tft.fillTriangle(87, 136, 84, 141, 90, 141, 0xd0a0);
    } else {
        tft.fillTriangle(87, 136, 84, 141, 90, 141, 0xffff);
        tft.drawTriangle(87, 136, 84, 141, 90, 141, 0xd0a0);
    }
    if (JoystickYDownState == HIGH) {
        tft.fillTriangle(87, 154, 84, 149, 90, 149, 0xd0a0);
    } else {
        tft.fillTriangle(87, 154, 84, 149, 90, 149, 0xffff);
        tft.drawTriangle(87, 154, 84, 149, 90, 149, 0xd0a0);
    }
}

void motorInit(void) {
    pinMode(37, OUTPUT);
    pinMode(39, OUTPUT);
    pinMode(41, OUTPUT);
    digitalWrite(37, LOW);
    digitalWrite(39, HIGH);
}
void motorON(void) { digitalWrite(41, HIGH); }
void motorOFF(void) { digitalWrite(41, LOW); }

void laserInit(void) {
    pinMode(D53, OUTPUT);
    pinMode(D51, OUTPUT);
    digitalWrite(D53, HIGH);
}
void laserON(void) {
    laserState = HIGH;
    digitalWrite(D51, laserState);
}
void laserOFF(void) {
    laserState = LOW;
    digitalWrite(D51, laserState);
}

void capacitiveInit(void) {
    pinMode(23, INPUT);
    pinMode(25, OUTPUT);
    digitalWrite(25, LOW);
}
void capacitiveUpdate(void) { capacitiveValue = digitalRead(23); }

void imuInit(void) {
    Wire.begin();
    // Activate the MPU-6050
    Wire.beginTransmission(0x68);  // Device Address
    Wire.write(0x6B);              // Register to write too
    Wire.write(0x00);              // Value to write
    Wire.endTransmission();

    // Set Accelerometer sensitivity
    // Wire.write; 2g -> 0x00, 4g -> 0x08, 8g -> 0x10, 16g -> 0x18
    Wire.beginTransmission(0x68);
    Wire.write(0x1C);
    Wire.write(0x08);
    Wire.endTransmission();

    // Set Gyro sensitivity
    // 250 deg/s -> 0x00, 500 deg/s -> 0x08, 1000 deg/s -> 0x10, 2000 deg/s -> 0x18
    Wire.beginTransmission(0x68);
    Wire.write(0x1B);
    Wire.write(0x08);
    Wire.endTransmission();
}

void imuGetData(void) {
    // Set the Register to read from
    Wire.beginTransmission(0x68);
    Wire.write(0x3B);
    Wire.endTransmission();

    // Request 14 bytes from MPU6050
    Wire.requestFrom(0x68, 14);

    // Read data 6x Acceleration Bytes, 2x Temp Bytes, 6x Gyro Bytes
    //               High Byte          Low Byte
    int16_t acc_x = (Wire.read() << 8 | Wire.read());
    int16_t acc_y = Wire.read() << 8 | Wire.read();
    int16_t acc_z = Wire.read() << 8 | Wire.read();
    int16_t temperature = Wire.read() << 8 | Wire.read();
    int16_t gyro_x = Wire.read() << 8 | Wire.read();
    int16_t gyro_y = Wire.read() << 8 | Wire.read();
    int16_t gyro_z = Wire.read() << 8 | Wire.read();

    // Acceleration Conversion
    float ax = acc_x / 8192.0;
    float ay = acc_y / 8192.0;
    float az = acc_z / 8192.0;

    // Temperature Conversion
    Temp = (temperature / 340.0) + 36.53;

    // Gyroscope Conversion
    // double gX = gyro_x/65.5;
    // double gY = gyro_y/65.5;
    // double gZ = gyro_z/65.5;

    // mpu.getMotion6(&ax, &ay, &az, &wx, &wy, &wz);
    // Temp=mpu.getTemperature()/340.0f + 36.53f;
    roll = 360.0 / 6.28 * atan2(ax, sqrtf(ay * ay + az * az));
    pitch = 360.0 / 6.28 * atan2(ay, sqrtf(ax * ax + az * az));
}

void joystickInit(void) {
    pinMode(BUTTON_UP, INPUT);
    pinMode(BUTTON_RIGHT, INPUT);
    pinMode(BUTTON_DOWN, INPUT);
    pinMode(BUTTON_LEFT, INPUT);
    pinMode(BUTTON_E, INPUT);
    pinMode(BUTTON_F, INPUT);
    pinMode(BUTTON_K, INPUT);
    pinMode(PIN_ANALOG_X, INPUT);
    pinMode(PIN_ANALOG_Y, INPUT);
}

void joystickGetData(void) {
    JoystickAnalogX = analogRead(PIN_ANALOG_X);
    JoystickAnalogY = analogRead(PIN_ANALOG_Y);

    ButtonUpState = digitalRead(BUTTON_UP);
    ButtonRightState = digitalRead(BUTTON_RIGHT);
    ButtonDownState = digitalRead(BUTTON_DOWN);
    ButtonLeftState = digitalRead(BUTTON_LEFT);
    ButtonEState = digitalRead(BUTTON_E);
    ButtonFState = digitalRead(BUTTON_F);
    ButtonKState = digitalRead(BUTTON_K);

    if (JoystickAnalogX > 512 + joystickDeadband) {
        x = x + 1;
        JoystickXRightState = HIGH;
        if (x > 100) x = 100;
    } else {
        JoystickXRightState = LOW;
    }

    if (JoystickAnalogX < 512 - joystickDeadband) {
        x = x - 1;
        JoystickXLeftState = HIGH;
        if (x < -100) x = -100;
    } else {
        JoystickXLeftState = LOW;
    }

    if (JoystickAnalogY > 512 + joystickDeadband) {
        y = y + 1;
        JoystickYUpState = HIGH;
        if (y > 100) y = 100;
    } else {
        JoystickYUpState = LOW;
    }

    if (JoystickAnalogY < 512 - joystickDeadband) {
        y = y - 1;
        JoystickYDownState = HIGH;
        if (y < -100) y = -100;
    } else {
        JoystickYDownState = LOW;
    }

    if (ButtonKState == RISING) {
        // x=0;
        // y=0;
    }

    if (ButtonUpState == LOW) {
        z = z + 1;
        if (z > 100) z = 100;
    }

    if (ButtonDownState == LOW) {
        z = z - 1;
        if (z < -100) z = -100;
    }

    if (ButtonRightState == LOW) {
        yaw = yaw + 1.0;
    }

    if (ButtonLeftState == LOW) {
        yaw = yaw - 1.0;
    }

    // memories
    xLast = x;
    yLast = y;
}

void displayInit(void) {
    display.begin();
    display.showString("Init");
}

void displayUpdate(void) {
    counter++;

    // secondRunning=(xTaskGetTickCount()/configTICK_RATE_HZ)%60;
    // minuteRunning=(xTaskGetTickCount()/configTICK_RATE_HZ)/60;
    auto timeSecondsNow = time_point_cast<seconds>(Kernel::Clock::now());  // Convert time_point to one in microsecond accuracy
    long timeSeconds = timeSecondsNow.time_since_epoch().count();
    secondRunning = timeSeconds % 60;
    minuteRunning = timeSeconds / 60;

    if (counter % 1 == 0) {
        displayDots = !displayDots;
    }
    // display.showNumberDec(int num, uint8_t dots = 0, bool leading_zero = false, uint8_t length = MAXDIGITS, uint8_t pos = 0);
    if ((minute != 0) && (hour != 0)) {
        display.showNumberDec(minute, 0b01000000 * displayDots, true, 2, 2);
        display.showNumberDec(hour, 0b01000000 * displayDots, true, 2, 0);
    } else {
        display.showNumberDec(secondRunning, 0b01000000 * displayDots, true, 2, 2);
        display.showNumberDec(minuteRunning, 0b01000000 * displayDots, true, 2, 0);
    }
}

void supervisionUpdate(void) {
    printMutex.lock();
    // Serial.println("OSC");
    // Serial.print(Temp);
    // Serial.print(",");
    // Serial.print(pitch);
    // Serial.print(",");
    // Serial.print(roll);
    // Serial.print(",");
    // Serial.print(lat);
    // Serial.print(",");
    // Serial.print(lon);
    // Serial.println(" ");
    printMutex.unlock();
}
