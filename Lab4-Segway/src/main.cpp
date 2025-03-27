#include <Arduino.h>
#include <WiFi.h>
#include "ArduinoJson.h"  //JSON packaging
#include "M5StickCPlus.h"

// Tenim el num 1 //

// char ssid[] = "WiFiAccessPointGiga_1";        // your network SSID (name)
// char password[] = "WiFiAccessPointGiga_1";        // your network password (use for WPA, or use as key for WEP)
// char ssid[] = "wlan_str";        // your network SSID (name)
// char password[] = "wlan_str";        // your network password (use for WPA, or use as key for WEP)
// IPAddress ipLocal=IPAddress(192, 168, 1, 102);//fixed ip not working...!!!

const float rZero = -99.75;  // reference pitch angle
float r = rZero;             // variable to deal with varying reference
float error;
float error_k_1 = 0;
float kp = 2.5;  // the magic gains
float ki = 75.00;
float kd = 25.0;
float ku = 0.6;
float P = 0;  // the controller
float I = 0;
float D = 0;
float u = 0;
float gX = 0;  // gyro data
float gY = 0;
float gZ = 0;
float aX = 0;  // acceleration data
float aY = 0;
float aZ = 0;
float xAcc = 0;
float yAcc = 0;
float zAcc = 0;
float xGyro = 0;
float yGyro = 0;
float zGyro = 0;
float xAccOffset = 0;
float yAccOffset = 0;
float zAccOffset = 0;
float xGyroOffset = 0;
float yGyroOffset = 0;
float zGyroOffset = 0;
float omega = 0;
float xOmega = 0;
float pitch = 0;
float roll = 0;
float pitch_filtered = 0;
float roll_filtered = 0;
float pit = 0;
float rol = 0;
float yaw = 0;
long int tStart = 0;
long int tElapsed = 0;
float h = 0.010;          // sampling time in sec
long int delayTime = 10;  // delay time in millisec
float vBatt = 0;

// Function prototypes
void initWifi(void);
void initM5StickCPlus(void);
void IMUCalibration(void);

void setup() {
    Serial.begin(115200);
    Serial.println("Init...");
    
    initM5StickCPlus();
}

void loop() {
    tStart = millis();

    // Task: ReadSensors
    M5.Imu.getGyroData(&gX, &gY, &gZ);   // gyroscope
    M5.Imu.getAccelData(&aX, &aY, &aZ);  // accelerometer

    xAcc = aX;
    yAcc = aY;
    zAcc = aZ;
    xGyro = gX - xGyroOffset;
    yGyro = gY - yGyroOffset;
    zGyro = gZ - zGyroOffset;
    // -xGyro*M5.IMU.gRes 
    // M5.IMU.gRes=0.01 https://docs.m5stack.switch-science.com/en/arduino/m5stickc/sh200q_m5stickc degrees per
    // second. TODO: check it
    xOmega = -xGyro * 0.01;

    roll = -360.0 / 6.28 * atan2(-xAcc, zAcc);
    pitch = 360.0 / 6.28 * atan2(yAcc, zAcc);
    // low pass filter + complementary filter (handmade). TODO: double check it!
    pitch_filtered = 0.993 * (pitch_filtered + xGyro * h) + (1.0 - 0.993) * (pitch);

    r = rZero;
    error = r - pitch_filtered;
    P = kp * error;
    I = I + ki * h * error;
    if (I > 100) I = 100;
    if (I < -100) I = -100;
    D = kd * xOmega;
    u = P + I + D + ku * u;

    // if the error is big, do nothing since the segway has fallen
    if (abs(error) > 30) {
        u = 0;
    }
    // check for saturation
    if (u > 127) u = 127;  
    if (u < -127) u = -127;

    // send data to motors
    Wire.beginTransmission(0x38);
    // channel left  
    Wire.write(0);                 
    Wire.write((int)(-u));
    Wire.endTransmission();
    Wire.beginTransmission(0x38);
    // channel right
    Wire.write(1);  
    Wire.write((int)(u));
    Wire.endTransmission();

    vBatt = M5.Axp.GetBatVoltage();
    M5.Lcd.setCursor(0, 10);
    M5.Lcd.printf("v=%5.2fV   ", vBatt);
    M5.Lcd.setCursor(0, 40);
    M5.Lcd.printf("r=%5.2f   ", r);
    M5.Lcd.setCursor(0, 70);
    M5.Lcd.printf("pitch=%d   ", (int)pitch_filtered);

    Serial.println("OSC");
    Serial.print((float)vBatt);
    Serial.print(",");
    Serial.print((float)r);
    Serial.print(",");
    Serial.print((float)pitch_filtered);
    Serial.print(",");
    Serial.print(delayTime);
    Serial.println();

    tElapsed = millis() - tStart;
    delayTime = (long int)(h * 1000.0f) - tElapsed;
    if (delayTime < 0) delayTime = 0;
    if (delayTime > 10) delayTime = 10;
    delay(delayTime);  // Periodic call
}

// IMUCalibration is only used to remove gyroscope offset
void IMUCalibration(void) {
    unsigned int calibrationIter = 10000;
    M5.Lcd.fillScreen(BLACK);
    Serial.println("Calibrating IMU... do not move the IMU");
    M5.Lcd.println("Cal.");
    for (int i = 0; i < calibrationIter; i++) {
        M5.Imu.getAccelData(&aX, &aY, &aZ);
        M5.Imu.getGyroData(&gX, &gY, &gZ);
        // M5.Imu.getAhrsData(&pit, &rol, &yaw);
        xAccOffset += aX;
        yAccOffset += aY;
        zAccOffset += aZ;
        xGyroOffset += gX;
        yGyroOffset += gY;
        zGyroOffset += gZ;
    }
    xAccOffset = xAccOffset / (float)calibrationIter;
    yAccOffset = yAccOffset / (float)calibrationIter;
    zAccOffset = zAccOffset / (float)calibrationIter - 1.0;
    xGyroOffset = xGyroOffset / (float)calibrationIter;
    yGyroOffset = yGyroOffset / (float)calibrationIter;
    zGyroOffset = zGyroOffset / (float)calibrationIter;
    Serial.println("IMU calibrated!");
    M5.Lcd.println("Done");
    M5.Lcd.fillScreen(BLACK);
}

void initM5StickCPlus(void) {
    pinMode(M5_LED, OUTPUT);
    pinMode(M5_BUTTON_HOME, INPUT);
    pinMode(M5_BUTTON_RST, INPUT);
    // pinMode(LEDC,OUTPUT);

    M5.begin();

    M5.Lcd.setRotation(2);
    M5.Lcd.setTextFont(4);
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setTextSize(1);
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setCursor(0, 210);
    M5.Lcd.printf("Init...");
    delay(1000);

    for (int i = 0; i < 2; i++) {
        M5.Beep.beep();
        delay(100);
        M5.Beep.mute();
        delay(100);
    }

    Wire.begin(0, 26);  // SDA,SCL
    // M5.Axp.ScreenBreath(11);

    M5.Imu.Init();
    M5.Imu.SetGyroFsr(M5.Imu.GFS_250DPS);  // 250DPS 500DPS 1000DPS 2000DPS
    M5.Imu.SetAccelFsr(M5.Imu.AFS_4G);     // 2G 4G 8G 16G

    // initWifi();

    IMUCalibration();
    M5.Lcd.fillScreen(BLACK);
}

void initWifi(void) {
    // // M5.Lcd.setCursor(0, 190);
    // M5.Lcd.setCursor(0, 0);
    // M5.Lcd.printf("Connecting...");
    // WiFi.mode(WIFI_STA);
    // WiFi.begin(ssid, password);
    // while (WiFi.waitForConnectResult() != WL_CONNECTED)
    // {
    //   Serial.println("Connection Failed! Rebooting...");
    //   delay(500);
    //   // M5.Lcd.setCursor(0, 190);
    //   M5.Lcd.setCursor(0, 190);
    //   M5.Lcd.printf("Rebooting... ");
    //   ESP.restart();
    // }
    // Serial.println("Ready");
    // Serial.print("IP address: ");

    // ipLocal=WiFi.localIP();
    // Serial.println(ipLocal);
    // // M5.Lcd.setCursor(0, 190);
    // M5.Lcd.setCursor(0, 0);
    // M5.Lcd.printf("%d.%d.%d.%d",ipLocal[0],ipLocal[1],ipLocal[2],ipLocal[3]);
    // M5.Lcd.setRotation(2);
    // delay(100);
    // Udp.begin(8888);
}


// /**
//  * @brief Get the time since system start in milliseconds
//  *
//  * @return float
//  */
// float str_getTime(void) {
//   float t = 10.0 * (float)(xTaskGetTickCount()) + 0.0005 * (float)TCNT1;
//   return t;
// }