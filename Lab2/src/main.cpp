#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include <timers.h>

void TaskBlink(void* params);
void TaskPrintTicks(void* args);
void SuspendTimerCallback(TimerHandle_t handle);

void setup() {
  Serial.begin(115200);
  Serial.println("Hello! :)");

  xTimerCreate("SoftwareTimer", pdMS_TO_TICKS(10000), pdFALSE /* one-shot*/, NULL, SuspendTimerCallback);

  xTaskCreate(
    TaskBlink,
    "Blink",
    128,
    NULL,
    1,
    NULL
  );
  xTaskCreate(
    TaskPrintTicks,
    "PrintTicks",
    128,
    NULL,
    2,
    NULL
  );
  
}

void loop() {
  // Serial.print("t=");
  // Serial.print(millis());
  // Serial.println();
}

void TaskBlink(void* args) {
  // Setup
  auto start = xTaskGetTickCount();
  pinMode(LED_BUILTIN, OUTPUT);

  // Loop
  for (;;) {
    digitalWrite(LED_BUILTIN, digitalRead(LED_BUILTIN) ^ 1);
    xTaskDelayUntil(&start, pdMS_TO_TICKS(500));
  }

  // traceTASK_SWITCHED_OUT();
}

void TaskPrintTicks(void* args) {
  // Setup
  auto start = xTaskGetTickCount();
  // Loop
  for (;;) {
    Serial.print("t=");
    Serial.print(start);
    Serial.print(" => ");
    Serial.print(start * portTICK_PERIOD_MS);
    Serial.print("ms");
    Serial.println();
    xTaskDelayUntil(&start, pdMS_TO_TICKS(200));
  }
}

void SuspendTimerCallback(TimerHandle_t handle) {
  // TODO
}

// Pregunta 4: static void prvSetupTimerInterrupt( void )


/*

In file included from lib\Arduino_FreeRTOS_Library-master\src\port.c:33:0:
lib\Arduino_FreeRTOS_Library-master\src\port.c: In function 'portSCHEDULER_ISR':        
lib\Arduino_FreeRTOS_Library-master\src\port.c:808:9: warning: 'portSCHEDULER_ISR' appears to be a misspelled 'signal' handler, missing '__vector' prefix [-Wmisspelled-isr]    
     ISR(portSCHEDULER_ISR)

*/