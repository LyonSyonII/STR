#include "Adafruit_GFX.h"     //TFT screen // Core graphics library
#include "Adafruit_ST7735.h"  // Hardware-specific library for ST7735
#include "Arduino.h"
#include "ArduinoJson.h"  //JSON packaging
#include "MPU6050.h"      //IMU Inertial Measurement Unit MPU6050
#include "RPC.h"
#include "TM1637TinyDisplay.h"  //TM1637 7-segment 4x display
#include "mbed.h"
#include "rtos.h"
#include "WiFi.h"

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

// thin-film-transistor liquid-crystal display
const uint8_t TFT_CS = D13;
const uint8_t TFT_DC = D11;
const uint8_t TFT_MOSI = D10;
const uint8_t TFT_SCLK = D9;
const uint8_t TFT_RST = D12;
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);

// WiFi for udp, ur3 comm, telemetry, http, etc.
const char ssid[] = "patata1234";  // Access point
const char pass[] = "patata1234";  // Access point
int status = WL_IDLE_STATUS;     // the WiFi radio's status
IPAddress ip(192, 168, 1, 111);  // static ip is not working!
WiFiUDP Udp;
IPAddress segwayIp(192, 168, 1, 112);
unsigned int segwayPort = 8888;

// a mutex used when printing
Mutex printMutex;  // Since we're printing from multiple threads, we need a mutex

void wifiInitAccessPoint(void);

Thread tftThread(osPriorityNormal);
void tftInit(void);
void tftTask(void);

// Thread joystickThread(osPriorityRealtime7);
void joystickInit(void);
void joystickTask(void);
void joystickGetData(void);
void sendCommand(uint8_t command);

Thread supervisionThread(osPriorityLow);
void supervisionTask(void);

Thread receiveDataThread(osPriorityRealtime7);
void receiveDataTask(void);

void setup() {
    Serial.begin(115200);
    Serial.println("Init...");

    wifiInitAccessPoint();
    tftInit();
    joystickInit();

    // ckThread.start(joystickTask);
    supervisionThread.start(supervisionTask);
    tftThread.start(tftTask);
    receiveDataThread.start(receiveDataTask);
}

void loop() {}

void receiveDataTask() {
    while (true) {
        const uint64_t lastWakeTime = Kernel::get_ms_count();

        int packetSize = Udp.parsePacket();

        if (packetSize) {
            Serial.print("Received data from ");
            Serial.print(Udp.remoteIP());
            Serial.print(":");
            Serial.print(Udp.remotePort());
            Serial.println();
            Serial.print("Received packet of size ");
            Serial.println(packetSize);
            char packetBuffer[255];
            int len = Udp.read(packetBuffer, 255);
            if (len > 0) {
                packetBuffer[len] = 0;
            }
            Serial.print("Contents: ");
            Serial.println(packetBuffer);
        }
        else {
            Serial.println("No packet received");
        }
    }
}

void joystickTask(void) {
    while (true) {
        const uint64_t lastWakeTime = Kernel::get_ms_count();
        joystickGetData();
        sendCommand(0xF1);
        ThisThread::sleep_until(50 + lastWakeTime);
    }
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

    Udp.begin(8888);
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

void tftTask(void) {
    while (true) {
        const uint64_t lastWakeTime = Kernel::get_ms_count();
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

        ThisThread::sleep_until(lastWakeTime + 100);
    }
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

void supervisionTask(void) {
    while (true) {
        const uint64_t lastWakeTime = Kernel::get_ms_count();
        printMutex.lock();
        Serial.println("OSC");
        Serial.print(Temp);
        Serial.print(",");
        Serial.print(pitch);
        Serial.print(",");
        Serial.print(roll);
        Serial.println(" ");
        printMutex.unlock();
        ThisThread::sleep_until(lastWakeTime + 200);
    }
}
