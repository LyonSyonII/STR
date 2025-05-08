#include <Arduino.h>
#include <WiFi.h>
#include "ArduinoJson.h"  //JSON packaging
#include "M5StickCPlus.h"

// Tenim el num 1 //

// char ssid[] = "WiFiAccessPointGiga_1";        // your network SSID (name)
// char password[] = "WiFiAccessPointGiga_1";        // your network password (use for WPA, or use as key for WEP)
char ssid[] = "patata1234";       // your network SSID (name)
char password[] = "patata1234";        // your network password (use for WPA, or use as key for WEP)
WiFiUDP Udp;
IPAddress ipLocal;
char moveMode = 'f';

struct Offsets {
    float xAccOffset;
    float yAccOffset;
    float zAccOffset;
    float xGyroOffset;
    float yGyroOffset;
    float zGyroOffset;
};

TaskHandle_t task1MoveMotorHandle;
TaskHandle_t task2ReceiveUDPHandle;
TaskHandle_t task9DebugHandle;

// float rZero = -99.75;  // reference pitch angle
// float rZero = -103.85;  // reference pitch angle
float rZero = -102.15;  // reference pitch angle
// float rZero = -101.75;  // reference pitch angle
float r = rZero;
float gX = 0;  // gyro data
float gY = 0;
float gZ = 0;
float aX = 0;  // acceleration data
float aY = 0;
float aZ = 0;
float vBatt = 0;
float pitch_filtered = 0;

// Function prototypes
void initWifi(void);
Offsets initM5StickCPlus(void);
Offsets IMUCalibration(void);

void task1MoveMotor(const Offsets& offsets);
void task2ReceiveUDP(void*);
void task9Debug(void*);

void setup() {
    Serial.begin(115200);
    Serial.println("Init...");
    
    Offsets offsets = initM5StickCPlus();

    xTaskCreate(
        (void(*)(void*))task1MoveMotor,
        "task1MoveMotor",
        configMINIMAL_STACK_SIZE*3,
        &offsets,
        9,
        &task1MoveMotorHandle
    );

    xTaskCreate(
        (void(*)(void*))task2ReceiveUDP,
        "task2ReceiveUDP",
        configMINIMAL_STACK_SIZE*3,
        NULL,
        8,
        &task2ReceiveUDPHandle
    );

    xTaskCreate(
        task9Debug,
        "task9Debug",
        configMINIMAL_STACK_SIZE*3,
        NULL,
        1,
        &task9DebugHandle
    );
}

void loop() {}

// C = 4ms
// T = 10ms
void task1MoveMotor(const Offsets &offsets) {    
    // setup
    TickType_t lastWake = 0;
    // long worst_exec_time = 0;
    
    const float h = 0.01; // 10 ms
    float kp = 2.5;  // the magic gains
    float ki = 75.00;
    float kd = 25.0;
    float ku = 0.6;
    
    float P = 0;  // the controller
    float I = 0;
    float D = 0;
    float u = 0;

    while (1) {
        // long start = millis();

        // Task: ReadSensors
        M5.Imu.getGyroData(&gX, &gY, &gZ);   // gyroscope
        M5.Imu.getAccelData(&aX, &aY, &aZ);  // accelerometer
    
        float xAcc = aX;
        float yAcc = aY;
        float zAcc = aZ;
        float xGyro = gX - offsets.xGyroOffset;
        float yGyro = gY - offsets.yGyroOffset;
        float zGyro = gZ - offsets.zGyroOffset;
        // -xGyro*M5.IMU.gRes 
        // M5.IMU.gRes=0.01 https://docs.m5stack.switch-science.com/en/arduino/m5stickc/sh200q_m5stickc degrees per
        // second. TODO: check it
        float xOmega = -xGyro * 0.01;
    
        float roll = -360.0 / 6.28 * atan2(-xAcc, zAcc);
        float pitch = 360.0 / 6.28 * atan2(yAcc, zAcc);
        // low pass filter + complementary filter (handmade). TODO: double check it!
        pitch_filtered = 0.993 * (pitch_filtered + xGyro * h) + (1.0 - 0.993) * (pitch);
    
        float error = r - pitch_filtered;
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
        // channel left  
        Wire.beginTransmission(0x38);
        Wire.write(0);                 
        Wire.write(moveMode != 'r' ? (int)(-u) : (int)(-u * .6));
        Wire.endTransmission();
        // channel right
        Wire.beginTransmission(0x38);
        Wire.write(1);  
        Wire.write(moveMode != 'l' ? (int)(u) : (int)(-u * .6));
        Wire.endTransmission();
        
        // auto elapsed = millis() - start;
        // if (elapsed > worst_exec_time) { 
        //     worst_exec_time = elapsed;
        //     printf("task1 took %lu ms; worst time: %lu\n", elapsed, worst_exec_time);
        // }
        
        vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(h * 1000.0f));
    }
}

// C = 2ms
// T = 5ms
void task2ReceiveUDP(void*) {
    TickType_t lastWake = 0;
    // long worst_exec_time = 0;

    while (1) {
        // long start = millis();

        int packet_available = Udp.parsePacket(); // Serial.printf("Checking Udp: %d\n", packet_available);
        if (packet_available) {
            signed char cmd;
            Udp.read((char*)&cmd, 1);
            
            if (cmd < 0) {
                const float step = 5.;
                const float range = 4.;
                r = (rZero - step * range) + step * (abs(cmd) - 1.);
                // -1, -2, -3, -4, rZero, -6, -7, -8, -9
                Serial.printf("%d\n", (int)cmd);
            } else {
                moveMode = cmd;
                Serial.printf("%c\n", cmd);
            }
        }

        // auto elapsed = millis() - start;
        // if (elapsed > worst_exec_time) { 
        //     worst_exec_time = elapsed;
        //     printf("task2 took %lu ms; worst time: %lu\n", elapsed, worst_exec_time);
        // }

        xTaskDelayUntil(&lastWake, pdMS_TO_TICKS(5));
    }
}

/*

// uint64_t worst_exec_time = 0;
const uint64_t start = Kernel::get_ms_count();
// uint64_t elapsed = Kernel::get_ms_count() - start;
// if (elapsed > worst_exec_time) { 
//     worst_exec_time = elapsed;
//     printf("task2 took %lu ms; worst time: %lu\n", elapsed, worst_exec_time);
// }
*/

// C = 104ms
// T = 200ms
void task9Debug(void*) {
    TickType_t lastWake = 0;
    long worst_exec_time = 0;

    while (1) {        
        long start = millis();
        vBatt = M5.Axp.GetBatVoltage();
        M5.Lcd.setCursor(0, 10);
        M5.Lcd.printf("v=%5.2fV   ", vBatt);
        M5.Lcd.setCursor(0, 40);
        M5.Lcd.printf("r=%5.2f   ", r);
        M5.Lcd.setCursor(0, 70);
        M5.Lcd.printf("pitch=%.2f   ", pitch_filtered);
        M5.Lcd.setCursor(0, 190);
        // M5.Lcd.printf("%d.%d.%d.%d",ipLocal[0],ipLocal[1],ipLocal[2],ipLocal[3]);

        Serial.printf("OSC\n%.2f,%.2f,%.2f\n", vBatt, r, pitch_filtered);

        // auto elapsed = millis() - start;
        // if (elapsed > worst_exec_time) { 
        //     worst_exec_time = elapsed;
        //     printf("task9 took %lu ms; worst time: %lu\n", elapsed, worst_exec_time);
        // }

        xTaskDelayUntil(&lastWake, pdMS_TO_TICKS(200));
    }
}

// IMUCalibration is only used to remove gyroscope offset
Offsets IMUCalibration(void) {
    Offsets offsets;

    const unsigned int calibrationIter = 100;
    M5.Lcd.fillScreen(BLACK);
    Serial.println("Calibrating IMU... do not move the IMU");
    M5.Lcd.println("Cal.");
    for (int i = 0; i < calibrationIter; i++) {
        M5.Imu.getAccelData(&aX, &aY, &aZ);
        M5.Imu.getGyroData(&gX, &gY, &gZ);
        // M5.Imu.getAhrsData(&pit, &rol, &yaw);
        offsets.xAccOffset += aX;
        offsets.yAccOffset += aY;
        offsets.zAccOffset += aZ;
        offsets.xGyroOffset += gX;
        offsets.yGyroOffset += gY;
        offsets.zGyroOffset += gZ;
    }
    offsets.xAccOffset = offsets.xAccOffset / (float)calibrationIter;
    offsets.yAccOffset = offsets.yAccOffset / (float)calibrationIter;
    offsets.zAccOffset = offsets.zAccOffset / (float)calibrationIter - 1.0;
    offsets.xGyroOffset = offsets.xGyroOffset / (float)calibrationIter;
    offsets.yGyroOffset = offsets.yGyroOffset / (float)calibrationIter;
    offsets.zGyroOffset = offsets.zGyroOffset / (float)calibrationIter;
    Serial.println("IMU calibrated!");
    M5.Lcd.println("Done");
    M5.Lcd.fillScreen(BLACK);

    return offsets;
}

Offsets initM5StickCPlus(void) {
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

    
    Offsets ret = IMUCalibration();
    M5.Lcd.fillScreen(BLACK);
    initWifi();
    return ret;
}

void initWifi(void) {
    M5.Lcd.setCursor(0, 190);
    M5.Lcd.printf("Connecting...");
    WiFi.mode(WIFI_STA);
    // WiFi.config(ipLocal, IPAddress(0, 0, 0, 0), IPAddress(0, 0, 0, 0));
    WiFi.begin(ssid, password);
    while (WiFi.waitForConnectResult() != WL_CONNECTED)
    {
      Serial.println("Connection Failed! Rebooting...");
      delay(500);
      M5.Lcd.setCursor(0, 190);
      M5.Lcd.printf("Rebooting... ");
      ESP.restart();
    }
    Serial.println("Ready");
    Serial.print("IP address: ");

    ipLocal=WiFi.localIP();
    Serial.println(ipLocal);
    M5.Lcd.fillRect(0, 190, 13*20, 40, BLACK);
    M5.Lcd.setCursor(0, 190);
    M5.Lcd.printf("%d.%d.\n%d.%d",ipLocal[0],ipLocal[1],ipLocal[2],ipLocal[3]);
    M5.Lcd.setRotation(2);
    delay(100);
    Udp.begin(8888);
    Udp.flush();
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