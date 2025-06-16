#include <Arduino.h>
#include "FreeRTOS.h"
#include "task.h"
const int LED1= 21;
const int COL1= 4;

int led1State=LOW;

void Task1( void *pvParameters );

void setup() // put your setup code here, to run once:
{
  pinMode(LED1,OUTPUT);
  pinMode(COL1, OUTPUT);
  digitalWrite(COL1,LOW);
  Serial.begin(115200);
  xTaskCreate(Task1,"Task1", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
  vTaskStartScheduler();  
}

void loop() // put your main code here, to run repeatedly:
{
  // led1State ^= 1;
  // digitalWrite(LED1,led1State);
  // Serial.print("t=");
  // Serial.println(millis());
  // delay(500);
}

void Task1(void *pvParameters)  // This is a task.
{
	(void) pvParameters;

	TickType_t xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();                      

	for (;;) // A Task shall never return or exit.
	{  
      Serial.print("Task1 @ ");
      Serial.print((int)(xTaskGetTickCount()));
      Serial.print(" ticks, ");
      Serial.print(millis());
      Serial.println(" ms");
      led1State= led1State^1;
      digitalWrite(LED1,led1State);

      vTaskDelayUntil( &xLastWakeTime, pdTICKS_TO_MS(500));
  }
}

void vApplicationStackOverflowHook( TaskHandle_t xTask, char * pcTaskName )
{
  Serial.println(pcTaskName);
}
