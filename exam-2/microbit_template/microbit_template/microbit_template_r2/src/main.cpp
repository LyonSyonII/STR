#include <Arduino.h>
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"

#define TSTOP 50000 //Time in milliseconds to stop the kernel

const int LED1 = 21;
const int COL1 = 4;

//matrix leds
const int ROW1_PORT_BIT = 21;
const int ROW2_PORT_BIT = 22;
const int ROW3_PORT_BIT = 23;
const int ROW4_PORT_BIT = 24;
const int ROW5_PORT_BIT = 25;

const int COL1_PORT_BIT = 4;
const int COL2_PORT_BIT = 7;
const int COL3_PORT_BIT = 3;
const int COL4_PORT_BIT = 6;
const int COL5_PORT_BIT = 10;
const int LEDMICRO = 28;

int led1State=LOW;

//circular buffer for debugging
#define BUFF_SIZE 500
float t[BUFF_SIZE] = {};
char circ_buffer1[BUFF_SIZE] = {};
char circ_buffer2[BUFF_SIZE] = {};
char circ_buffer3[BUFF_SIZE] = {};
char circ_buffer4[BUFF_SIZE] = {};
char circ_buffer5[BUFF_SIZE] = {};
char circ_buffer6[BUFF_SIZE] = {};
char circ_buffer7[BUFF_SIZE] = {};
float debug_data1[BUFF_SIZE] = {};
unsigned int circ_buffer_counter = 0;

//task handlers
TaskHandle_t Task1Handle;
TaskHandle_t Task2Handle;
TaskHandle_t Task3Handle;

//timer handlers
TimerHandle_t xPeriodicTimer, xOneShotTimer;
BaseType_t xPeriodicTimerStarted, xOneShotStarted;

void Task1( void *pvParameters );
void Task2( void *pvParameters );
void Task3( void *pvParameters );
void OneShotTimerCallback( TimerHandle_t xTimer );

void str_compute(unsigned long milliseconds);
void str_trace(void);

void setup() // put your setup code here, to run once:
{
  pinMode(LED1,OUTPUT);
  pinMode(COL1, OUTPUT);
  digitalWrite(COL1,LOW);
  Serial.begin(115200);

  xOneShotTimer = xTimerCreate("OneShotTimer", pdMS_TO_TICKS(TSTOP)  , pdFALSE, 0, OneShotTimerCallback );
  xOneShotStarted = xTimerStart( xOneShotTimer, 0 );

  xTaskCreate(Task1,"Task1", configMINIMAL_STACK_SIZE, NULL, 3, &Task1Handle);
  xTaskCreate(Task2,"Task2", configMINIMAL_STACK_SIZE, NULL, 2, &Task2Handle);
  xTaskCreate(Task3,"Task3", configMINIMAL_STACK_SIZE, NULL, 1, &Task3Handle);
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
	xLastWakeTime = 0;	

	for (;;) // A Task shall never return or exit.
	{  
      Serial.print("Task1 @ ");
      Serial.print((int)(xTaskGetTickCount()));
      Serial.print(" ticks, ");
      Serial.print(millis());
      Serial.println(" ms");
      led1State= led1State^1;
      digitalWrite(LED1,led1State);

      str_compute(5);
      vTaskDelayUntil( &xLastWakeTime, pdTICKS_TO_MS(100));
  }
}

void Task2(void *pvParameters)  
{
	(void) pvParameters;

	TickType_t xLastWakeTime;
	xLastWakeTime = 0;

	for (;;) 
	{  
      str_compute(10);
      vTaskDelayUntil( &xLastWakeTime, pdTICKS_TO_MS(200));
  }
}

void Task3(void *pvParameters)  
{
	(void) pvParameters;

	TickType_t xLastWakeTime;
	xLastWakeTime = 0;

	for (;;) 
	{  
      str_compute(50);
      vTaskDelayUntil( &xLastWakeTime, pdTICKS_TO_MS(300));
  }
}

void vApplicationStackOverflowHook( TaskHandle_t xTask, char * pcTaskName )
{
  Serial.println(pcTaskName);
}


void OneShotTimerCallback( TimerHandle_t xTimer )
{
	TickType_t xTimeNow;
	xTimeNow = xTaskGetTickCount();

	str_trace();
	//Stop the kernel...
	vTaskSuspendAll();
	str_trace();
  vPortEndScheduler();

	//...and sent data to the host PC
	unsigned int i;
	for (i = 0; i < BUFF_SIZE; i++)
	{
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
		Serial.write((uint8_t)circ_buffer7[i]);
		Serial.print(",");
		Serial.print((float)debug_data1[i]);
		Serial.println();
	}
}

//str_getTime is a custom implementation of the time to debug data
float str_getTime(void)
{
// clear all;
// SystemCoreClock=64000000%From Serial.println(SystemCoreClock);
// configSYSTICK_CLOCK_HZ=SystemCoreClock
// configTICK_RATE_HZ=100;
// portNVIC_SYSTICK_LOAD_REG = ( configSYSTICK_CLOCK_HZ / configTICK_RATE_HZ );% - 1
// tick2milliseconds=1/configTICK_RATE_HZ*1000/portNVIC_SYSTICK_LOAD_REG
// vpa(tick2milliseconds,20)

  // #define portNVIC_SYSTICK_CURRENT_VALUE_REG    ( *( ( volatile uint32_t * ) 0xe000e018 ) )
  volatile uint32_t portNVIC_SYSTICK_CURRENT_VALUE_REG = ( *( ( volatile uint32_t * ) 0xe000e018 ) );

	// float t=micros();//ok
  // float t=millis();//not so precise
   float t=0.0000015625*((640000-portNVIC_SYSTICK_CURRENT_VALUE_REG)+xTaskGetTickCount()*640000);
  //(float)(0.5e-3*((float)OCR1A*xTaskGetTickCount()+TCNT1));//Sent time in milliseconds!!!
	return t;
}

//str_compute(x) is only used to waste time without using delays
void str_compute(unsigned long milliseconds)
{
  unsigned int i = 0;
  unsigned int imax = 0;
  imax = milliseconds * 3275;
  volatile float dummy = 1;
  for (i = 0; i < imax; i++)
  {
    dummy = dummy * dummy;
  }
}

//str_trace is a hook by the RTOS kernel used after a context-switch-in
void str_trace(void)
{
	circ_buffer_counter++;
	if (circ_buffer_counter >= BUFF_SIZE)
	{
		circ_buffer_counter = 0;
	}

	t[circ_buffer_counter] = str_getTime();//sent time in milliseconds
	circ_buffer1[circ_buffer_counter] = eTaskGetState(Task1Handle);
	circ_buffer2[circ_buffer_counter] = eTaskGetState(Task2Handle);
	circ_buffer3[circ_buffer_counter] = eTaskGetState(Task3Handle);
	circ_buffer4[circ_buffer_counter] = 0;
	circ_buffer5[circ_buffer_counter] = 0;
	circ_buffer6[circ_buffer_counter] = 0;
	circ_buffer7[circ_buffer_counter] = 0;
	debug_data1[circ_buffer_counter] = 1.2;
}


