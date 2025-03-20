#include <Arduino.h>

#include "Arduino_FreeRTOS.h"
#include "config/config.hh"
#include "timers.h"

// circular buffer for debugging
#define BUFF_SIZE 250

float t[BUFF_SIZE] = {};
byte circ_buffer1[BUFF_SIZE] = {};
byte circ_buffer2[BUFF_SIZE] = {};
byte circ_buffer3[BUFF_SIZE] = {};
byte circ_buffer9[BUFF_SIZE] = {};
float debug_data1[BUFF_SIZE] = {};
unsigned int circ_buffer_counter = 0;

// Task handlers
TaskHandle_t Task1ReadHallHandle;
TaskHandle_t Task2MoveMotorHandle;
TaskHandle_t Task3UpdateRefHandle;
TaskHandle_t Task9TraceHandle;

// Timer handlers
TimerHandle_t xOneShotTimer;
BaseType_t xOneShotStarted;

// WakeUpTime
TickType_t xLastWakeTime1;
TickType_t xLastWakeTime2;
TickType_t xLastWakeTime3;
TickType_t xLastWakeTime9;

// Function prototypes
void Task1ReadHall(void *pvParameters);
void InterruptReadHallA();
void InterruptReadHallB();

void Task2MoveMotor(void *pvParameters);
double Task2PID(int8_t ref, int16_t angleMesurat);

void Task3UpdateRef(void *pvParameters);

void Task9Trace(void *pvParameters);

void OneShotTimerCallback(TimerHandle_t xTimer);
void str_trace(void);
void str_compute(unsigned int);
float str_getTime(void);

void setup() {
  Serial.begin(115200);
  Serial.println("=== START ===");

  pinMode(PWM_A, OUTPUT);         // rotation speed (pwm)
  pinMode(DIR_A, OUTPUT);         // direction ()
  pinMode(HALL_A, INPUT_PULLUP);  // pin hall effect
  pinMode(HALL_B, INPUT_PULLUP);  // pin hall effect 2

  attachInterrupt(digitalPinToInterrupt(HALL_A), InterruptReadHallA, CHANGE);
  attachInterrupt(digitalPinToInterrupt(HALL_B), InterruptReadHallB, CHANGE);

  // fer vTaskResume des de interrupcio ISR per tal de cridar la nostra tasca
  // Per tant, la tasca de llegir es aperiodica (pero tant petita que no
  // importa)

  xOneShotTimer = xTimerCreate("OneShotTimer", pdMS_TO_TICKS(execution_time), pdFALSE, 0, OneShotTimerCallback);
  xOneShotStarted = xTimerStart(xOneShotTimer, 0);

  xTaskCreate(Task1ReadHall, "Task1ReadHall", configMINIMAL_STACK_SIZE, NULL, 9, &Task1ReadHallHandle);
  xTaskCreate(Task2MoveMotor, "Task2MoveMotor", configMINIMAL_STACK_SIZE, NULL, 8, &Task2MoveMotorHandle);
  xTaskCreate(Task3UpdateRef, "Task3UpdateRef", configMINIMAL_STACK_SIZE, NULL, 7, &Task3UpdateRefHandle);
  xTaskCreate(Task9Trace, "Task9Trace", configMINIMAL_STACK_SIZE, NULL, 6, &Task9TraceHandle);

  // Initialise the xLastWakeTime variable with the current time.
  xLastWakeTime1 = 0;
  xLastWakeTime2 = xLastWakeTime1;
  xLastWakeTime3 = xLastWakeTime1;
  xLastWakeTime9 = xLastWakeTime1;

  // This task is waken by hardware interrupts
  vTaskSuspend(Task1ReadHallHandle);

  // vTaskStartScheduler(); //Most ports require calling this to start the kernel
}

void loop() {}

void Task1ReadHall(void *pvParameters) {
  const uint8_t channelPinA = HALL_A;
  const uint8_t channelPinB = HALL_B;

  for (;;) {
    bool arePinsEqual = digitalRead(channelPinA) == digitalRead(channelPinB);

    is_motor_clockwise =
        ((Task1RunningPin == 1) && arePinsEqual) || ((Task1RunningPin == 2) && !arePinsEqual);

    if (is_motor_clockwise) {
      Task1HallCounter = (Task1HallCounter + 1) % ppr;
    } else {
      Task1HallCounter = (Task1HallCounter - 1) % ppr;
    }

    Task1RunningPin = 0;

    vTaskSuspend(Task1ReadHallHandle);
  }
}

void InterruptReadHallA() {
  if (!Task1RunningPin && xTaskResumeFromISR(Task1ReadHallHandle)) {
    Task1RunningPin = 1;
    vPortYieldFromISR();
  }
}

void InterruptReadHallB() {
  if (!Task1RunningPin && xTaskResumeFromISR(Task1ReadHallHandle)) {
    Task1RunningPin = 2;
    vPortYieldFromISR();
  }
}

void Task2MoveMotor(void *pvParameters) {
  for (;;) {
    int16_t angleMesurat = Task1HallCounter * hall_delta;
    double pwm = Task2PID(reference_angle, angleMesurat);

    digitalWrite(DIR_A, pwm < 0 ? 1 : 0);
    pwm = abs(pwm);
    if (pwm > 254) pwm = 254;
    else if (pwm < 74) pwm = 0;
    analogWrite(PWM_A, abs(pwm));
    vTaskDelayUntil(&xLastWakeTime2, pdMS_TO_TICKS(10));
  }
}

double Task2PID(int8_t ref, int16_t angleMesurat) {
  const float Kp = 10;
  const float Ki = 0;
  const float Kd = 0;
  const uint8_t Tpid = 10;
  static int16_t lastError = 0;
  static float I = 0;

  float error = (int16_t) ref - angleMesurat;
  float P = Kp * error;
  I += Ki * Tpid * error;
  float D = Kd * (error - lastError) / (float)Tpid;
  return P + I + D;
}

void Task3UpdateRef(void *pvParameters) {
  for (;;) {
    reference_angle *= -1;
    vTaskDelayUntil(&xLastWakeTime3, pdMS_TO_TICKS(1000));
  }
}

void Task9Trace(void *pvParameters) {
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
    Serial.print(str_getTime());
    Serial.print(",");
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
    vTaskDelayUntil(&xLastWakeTime9, pdMS_TO_TICKS(200));
  }
}

void OneShotTimerCallback(TimerHandle_t xTimer) {
  TickType_t xTimeNow = xTaskGetTickCount();
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
    Serial.write((uint8_t)circ_buffer9[i]);
    Serial.print(",");
    Serial.print((float)debug_data1[i]);
    Serial.println();
  }

  analogWrite(PWM_A, 0);
  Serial.println("=== END ===");
}

void str_trace(void) {
  circ_buffer_counter++;
  if (circ_buffer_counter >= BUFF_SIZE) {
    circ_buffer_counter = 0;
  }

  t[circ_buffer_counter] = str_getTime();  // sent time in milliseconds
  circ_buffer1[circ_buffer_counter] = eTaskGetState(Task1ReadHallHandle);
  circ_buffer2[circ_buffer_counter] = eTaskGetState(Task2MoveMotorHandle);
  circ_buffer3[circ_buffer_counter] = eTaskGetState(Task3UpdateRefHandle);
  circ_buffer9[circ_buffer_counter] = eTaskGetState(Task9TraceHandle);
  debug_data1[circ_buffer_counter] = 2.7;
}

/**
 * @brief Compute a dummy operation. Done to simulate a task
 *
 * @param milliseconds Time to waste in milliseconds
 */
void str_compute(unsigned int milliseconds) {
  unsigned int i = 0;
  unsigned int imax = 0;
  imax = milliseconds * 92;
  volatile float dummy = 1;
  for (i = 0; i < imax; i++) {
    dummy = dummy * dummy;
  }
}

/**
 * @brief Get the time since system start in milliseconds
 *
 * @return float
 */
float str_getTime(void) {
  float t = 10.0 * (float)(xTaskGetTickCount()) + 0.0005 * (float)TCNT1;
  return t;
}
