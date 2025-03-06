#include <Arduino.h>

#include "Arduino_FreeRTOS.h"
#include "timers.h"

// circular buffer for debugging
#define BUFF_SIZE 250

const byte DIR_A = 12;
const byte PWM_A = 3;
const byte HALL_A = 18;
const byte HALL_B = 19;

float t[BUFF_SIZE] = {};
byte circ_buffer1[BUFF_SIZE] = {};
byte circ_buffer2[BUFF_SIZE] = {};
byte circ_buffer3[BUFF_SIZE] = {};
byte circ_buffer4[BUFF_SIZE] = {};
byte circ_buffer5[BUFF_SIZE] = {};
byte circ_buffer6[BUFF_SIZE] = {};
float debug_data1[BUFF_SIZE] = {};
unsigned int circ_buffer_counter = 0;

// Task handlers
TaskHandle_t Task1Handle;
TaskHandle_t Task2Handle;
TaskHandle_t Task3Handle;
TaskHandle_t Task4Handle;
TaskHandle_t Task5Handle;
TaskHandle_t Task6Handle;

// Timer handlers
TimerHandle_t xOneShotTimer;
BaseType_t xOneShotStarted;

// WakeUpTime
TickType_t xLastWakeTime1;
TickType_t xLastWakeTime2;
TickType_t xLastWakeTime3;
TickType_t xLastWakeTime4;
TickType_t xLastWakeTime5;
TickType_t xLastWakeTime6;

// Function prototypes
void Task1MoveMotor(void *pvParameters);
void Task2PID(void *pvParameters);
void Task3(void *pvParameters);
void Task4(void *pvParameters);
void Task5(void *pvParameters);
void Task6Trace(void *pvParameters);
void OneShotTimerCallback(TimerHandle_t xTimer);
void str_trace(void);
void str_compute(unsigned int);
float str_getTime(void);

void setup() {
  Serial.begin(115200);

  pinMode(PWM_A, OUTPUT); // rotation speed (pwm)
  pinMode(DIR_A, OUTPUT); // direction ()
  pinMode(HALL_A, INPUT_PULLUP); // pin hall effect
  pinMode(HALL_B, INPUT_PULLUP); // pin hall effect 2


  // fer vTaskResume des de interrupcio ISR per tal de cridar la nostra tasca
  // Per tant, la tasca de llegir es aperiodica (pero tant petita que no importa) 

  // xOneShotTimer = xTimerCreate("OneShotTimer", pdMS_TO_TICKS(10000), pdFALSE, 0, OneShotTimerCallback);
  // xOneShotStarted = xTimerStart(xOneShotTimer, 0);

  xTaskCreate(Task1MoveMotor, "Task1MoveMotor", configMINIMAL_STACK_SIZE, NULL, 8, &Task1Handle);
  xTaskCreate(Task2PID, "Task2PID", configMINIMAL_STACK_SIZE, NULL, 6, &Task2Handle);
  xTaskCreate(Task3, "Task3", configMINIMAL_STACK_SIZE, NULL, 7, &Task3Handle);
  xTaskCreate(Task4, "Task4", configMINIMAL_STACK_SIZE, NULL, 5, &Task4Handle);
  xTaskCreate(Task5, "Task5", configMINIMAL_STACK_SIZE, NULL, 4, &Task5Handle);
  xTaskCreate(Task6Trace, "Task6Trace", configMINIMAL_STACK_SIZE, NULL, 3, &Task6Handle);

  // Initialise the xLastWakeTime variable with the current time.
  xLastWakeTime1 = 0;
  xLastWakeTime2 = xLastWakeTime1;
  xLastWakeTime3 = xLastWakeTime1;
  xLastWakeTime4 = xLastWakeTime1;
  xLastWakeTime5 = xLastWakeTime1;
  xLastWakeTime6 = xLastWakeTime1;
  
  // vTaskStartScheduler(); //Most ports require calling this to start the
  // kernel
}

void loop() {}

void Task1MoveMotor(void *pvParameters) {
  uint16_t newAngle = 0;
  digitalWrite(DIR_A, digitalRead(DIR_A) ^ 1);
  for (;;) {
    // newAngle = (newAngle + 1) % 360;
    // analogWrite(PWM_A, 0);
    // digitalWrite(LED_BUILTIN, digitalRead(LED_BUILTIN) ^ 1);
    // str_compute(10);
    analogWrite(PWM_A, 90);
    vTaskDelayUntil(&xLastWakeTime1, pdMS_TO_TICKS(500));
    
    analogWrite(PWM_A, 0);
    vTaskDelayUntil(&xLastWakeTime1, pdMS_TO_TICKS(2000));

    // vTaskDelayUntil(&xLastWakeTime1, pdMS_TO_TICKS(500));
    // analogWrite(PWM_A, 180);

    // vTaskDelayUntil(&xLastWakeTime1, pdMS_TO_TICKS(500));
    // analogWrite(PWM_A, 270);
    // vTaskDelayUntil(&xLastWakeTime1, pdMS_TO_TICKS(500));
    // analogWrite(PWM_A, 90);
  }
}

double PID(uint8_t ref, int16_t angleMesurat) {
  const double Kp = 0;
  const double Ki = 0;
  const double Kd = 0;
  const uint8_t Tpid = 0;
  static int16_t lastError = 0;
  static double I = 0;

  int16_t error = ref - angleMesurat;
  double P = Kp * error;
  I += Ki * Tpid * error;
  double D = Kd * (error - lastError) / Tpid;
  return P + I + D; 
}
void Task2PID(void *pvParameters) {
  // ref = alternant entre -90 i 90 cada segon
  // error = ref - angleMesurat
  // P = Kp * error
  // I += Ki * Tpid * error
  // D = Kd * (error - last_error) / Tpid

  // u = P + I + D

  // Signe => Direccio
  // Modul => PWM

  for (;;) {
    // str_compute(15);
    PID(0, 0);
    vTaskDelayUntil(&xLastWakeTime2, pdMS_TO_TICKS(100));
  }
}

void Task3(void *pvParameters) {
  for (;;) {
    str_compute(4);
    vTaskDelayUntil(&xLastWakeTime3, pdMS_TO_TICKS(50));
  }
}

void Task4(void *pvParameters) {
  for (;;) {
    str_compute(6);
    vTaskDelayUntil(&xLastWakeTime4, pdMS_TO_TICKS(150));
  }
}

void Task5(void *pvParameters) {
  for (;;) {
    str_compute(8);
    vTaskDelayUntil(&xLastWakeTime5, pdMS_TO_TICKS(100));
  }
}

void Task6Trace(void *pvParameters) {
  pinMode(A1, INPUT);
  pinMode(A2, INPUT);
  pinMode(A3, INPUT);
  pinMode(A4, INPUT);
  pinMode(A5, INPUT);

  for (;;) {
    int adcA1 = analogRead(A1);
    int adcA2 = analogRead(A2);
    int adcA3 = analogRead(A3);
    int adcA4 = analogRead(A4);
    int adcA5 = analogRead(A5);
    Serial.println("OSC");
    Serial.print(adcA1);
    Serial.print(",");
    Serial.print(adcA2);
    Serial.print(",");
    Serial.print(adcA3);
    Serial.print(",");
    Serial.print(adcA4);
    Serial.print(",");
    Serial.print(adcA5);
    Serial.println();
    str_compute(54);
    vTaskDelayUntil(&xLastWakeTime6, pdMS_TO_TICKS(200));
  }
}

void OneShotTimerCallback(TimerHandle_t xTimer) {
  TickType_t xTimeNow;
  xTimeNow = xTaskGetTickCount();
  // oneshottimer_count++;
  // stop the kernel...
  //  vTaskSuspend(Task1Handle);
  //  vTaskSuspend(Task2Handle);
  //  vTaskSuspend(Task3Handle);
  //  vTaskSuspend(Task4Handle);
  //  vTaskSuspend(Task5Handle);
  //  vTaskSuspend(Task6Handle);
  vTaskSuspendAll();

  //...and sent data to the host PC
  unsigned int i;
  for (i = 0; i < BUFF_SIZE; i++) {
    Serial.println("DAT");
    Serial.print((float)t[i]);
    Serial.print(",");
    Serial.write((uint8_t)circ_buffer1[i]);
    Serial.print(",");
    Serial.write((uint8_t)circ_buffer2[i]);
    Serial.print(",");
    Serial.write((uint8_t)circ_buffer3[i]);
    Serial.print(",");
    Serial.write((uint8_t)circ_buffer4[i]);
    Serial.print(",");
    Serial.write((uint8_t)circ_buffer5[i]);
    Serial.print(",");
    Serial.write((uint8_t)circ_buffer6[i]);
    Serial.print(",");
    Serial.print((float)debug_data1[i]);
    Serial.println();
  }
}

void str_trace(void) {
  circ_buffer_counter++;
  if (circ_buffer_counter >= BUFF_SIZE) {
    circ_buffer_counter = 0;
  }

  t[circ_buffer_counter] = str_getTime();  // sent time in milliseconds
  circ_buffer1[circ_buffer_counter] = eTaskGetState(Task1Handle);
  circ_buffer2[circ_buffer_counter] = eTaskGetState(Task2Handle);
  circ_buffer3[circ_buffer_counter] = eTaskGetState(Task3Handle);
  circ_buffer4[circ_buffer_counter] = eTaskGetState(Task4Handle);
  circ_buffer5[circ_buffer_counter] = eTaskGetState(Task5Handle);
  circ_buffer6[circ_buffer_counter] = eTaskGetState(Task6Handle);
  debug_data1[circ_buffer_counter] = 2.7;
}

// str_compute(x) is only used to waste time without using delays
void str_compute(unsigned int milliseconds) {
  unsigned int i = 0;
  unsigned int imax = 0;
  imax = milliseconds * 92;
  volatile float dummy = 1;
  for (i = 0; i < imax; i++) {
    dummy = dummy * dummy;
  }
}

float str_getTime(void) {
  // float t=(float)(0.5e-3*((float)OCR1A*xTaskGetTickCount()+TCNT1));//Sent
  // time in milliseconds!!!
  float t = 10.0 * (float)(xTaskGetTickCount()) + 0.0005 * (float)TCNT1;
  return t;
}
