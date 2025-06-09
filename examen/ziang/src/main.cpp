#include <Arduino.h>

#include "FreeRTOS.h"
#include "task.h"

const int LED1 = 21;
const int COL1 = 4;

int led1State = LOW;

void Task1(void *pvParameters);

void setup() {
    pinMode(LED1, OUTPUT);
    pinMode(COL1, OUTPUT);
    digitalWrite(COL1, LOW);
    Serial.begin(115200);
    xTaskCreate(Task1, "Task1", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
    vTaskStartScheduler();
}

void loop() {}

void Task1(void *pvParameters) {
    (void)pvParameters;

    TickType_t xLastWakeTime;
    xLastWakeTime = xTaskGetTickCount();

    for (;;) {
        Serial.print("Task1 @ ");
        Serial.print((int)(xTaskGetTickCount()));
        Serial.print(" ticks, ");
        Serial.print(millis());
        Serial.println(" ms");
        led1State = led1State ^ 1;
        digitalWrite(LED1, led1State);

        vTaskDelayUntil(&xLastWakeTime, pdTICKS_TO_MS(500));
    }
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {
    Serial.println(pcTaskName);
}
