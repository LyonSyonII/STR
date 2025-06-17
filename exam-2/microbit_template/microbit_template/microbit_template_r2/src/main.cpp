#include <Arduino.h>

#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"

// #define TSTOP 50000  // Time in milliseconds to stop the kernel
#define TSTOP 1000  // Time in milliseconds to stop the kernel

const int LED1 = 21;
const int COL1 = 4;

// matrix leds
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

int led1State = LOW;

// circular buffer for debugging
const uint8_t N_TASKS = 6;
const uint8_t N_SCHED_TASKS = N_TASKS - 1;

const size_t BUFF_SIZE = TSTOP * 5;
float t[BUFF_SIZE] = { };
char circ_buffers[N_TASKS][BUFF_SIZE] = { };
unsigned int circ_buffer_counter = 0;

// task handlers
typedef struct {
    const float deadline;
    const float period;
    float next_deadline;
} TaskEDF_t;

float systemStartupTime;
float maxTraceTime = INT32_MIN;
float accTraceTime = 0;
float maxSchedTime = INT32_MIN;
float accSchedTime = 0;

TaskHandle_t TaskHandles[N_TASKS];
TaskEDF_t TaskEDF[N_SCHED_TASKS] = {
    {
        .deadline = 15,
        .period = 30,
        .next_deadline = 15,
    },
    {
        .deadline = 20,
        .period = 30,
        .next_deadline = 15,
    },
    {
        .deadline = 35,
        .period = 40,
        .next_deadline = 15,
    },
    {
        .deadline = 40,
        .period = 50,
        .next_deadline = 15,
    },
    {
        .deadline = 50,
        .period = 50,
        .next_deadline = 15,
    },
};


// timer handlers
TimerHandle_t xPeriodicTimer, xOneShotTimer;
BaseType_t xPeriodicTimerStarted, xOneShotStarted;

void Task1(void* pvParameters);
void Task2(void* pvParameters);
void Task3(void* pvParameters);
void Task4(void* pvParameters);
void Task5(void* pvParameters);
void Task9(void* pvParameters);
void OneShotTimerCallback(TimerHandle_t xTimer);

void str_compute(unsigned long milliseconds);
void str_trace(void);
float str_getTime(void);

void setup()  // put your setup code here, to run once:
{
    pinMode(LED1, OUTPUT);
    pinMode(COL1, OUTPUT);
    digitalWrite(COL1, LOW);
    Serial.begin(115200);
    
    xTaskCreate(Task1, "Task1", configMINIMAL_STACK_SIZE, NULL, 1, &TaskHandles[0]);
    xTaskCreate(Task2, "Task2", configMINIMAL_STACK_SIZE, NULL, 1, &TaskHandles[1]);
    xTaskCreate(Task3, "Task3", configMINIMAL_STACK_SIZE, NULL, 1, &TaskHandles[2]);
    xTaskCreate(Task4, "Task4", configMINIMAL_STACK_SIZE, NULL, 1, &TaskHandles[3]);
    xTaskCreate(Task5, "Task5", configMINIMAL_STACK_SIZE, NULL, 1, &TaskHandles[4]);
    xTaskCreate(Task9, "Task9", configMINIMAL_STACK_SIZE, NULL, configMAX_PRIORITIES-1, &TaskHandles[N_TASKS-1]);

    for (uint8_t i = 1; i < N_SCHED_TASKS; i++) {
        vTaskSuspend(TaskHandles[i]);
    }

    xOneShotTimer = xTimerCreate("OneShotTimer", pdMS_TO_TICKS(TSTOP), pdFALSE, 0, OneShotTimerCallback);
    xOneShotStarted = xTimerStart(xOneShotTimer, 0);

    vTaskStartScheduler();
}

void loop()  // put your main code here, to run repeatedly:
{
    // led1State ^= 1;
    // digitalWrite(LED1,led1State);
    // Serial.print("t=");
    // Serial.println(millis());
    // delay(500);
}

/// C = 2 ms
/// D = 15 ms
/// P = 30 ms
void Task1(void* pvParameters) {
    (void)pvParameters;
    TaskEDF_t *const task = &TaskEDF[0];

    TickType_t xLastWakeTime = xTaskGetTickCount();

    for (;;) {
        task->next_deadline = str_getTime() + task->deadline;
        str_compute(2);

        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(30));
    }
}

/// C = 4 ms
/// D = 20 ms
/// P = 30 ms
void Task2(void* pvParameters) {
    (void)pvParameters;
    TaskEDF_t *const task = &TaskEDF[1];

    TickType_t xLastWakeTime = xTaskGetTickCount();

    for (;;) {
        task->next_deadline = str_getTime() + task->deadline;
        str_compute(4);

        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(30));
    }
}

/// C = 10 ms
/// D = 35 ms
/// P = 40 ms
void Task3(void* pvParameters) {
    (void)pvParameters;
    TaskEDF_t *const task = &TaskEDF[2];

    TickType_t xLastWakeTime = xTaskGetTickCount();

    for (;;) {
        task->next_deadline = str_getTime() + task->deadline;
        str_compute(10);
        
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(40));
    }
}

/// C = 21 ms
/// D = 40 ms
/// P = 50 ms
void Task4(void* pvParameters) {
    (void)pvParameters;
    TaskEDF_t *const task = &TaskEDF[3];

    TickType_t xLastWakeTime = xTaskGetTickCount();

    for (;;) {
        task->next_deadline = str_getTime() + task->deadline;
        str_compute(21);

        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(50));
    }
}

/// C = 5 ms
/// D = 50 ms
/// P = 50 ms
void Task5(void* pvParameters) {
    (void)pvParameters;
    TaskEDF_t *const task = &TaskEDF[4];

    TickType_t xLastWakeTime = xTaskGetTickCount();

    for (;;) {
        task->next_deadline = str_getTime() + task->deadline;
        str_compute(5);

        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(50));
    }
}

void Task9(void* pvParameters) {
    (void)pvParameters;

    systemStartupTime = str_getTime();

    TickType_t xLastWakeTime = xTaskGetTickCount();

    // task1 will always be first
    uint8_t previous_task = 0;

    // find task with earliest deadline
    for (;;) {
        float startTime = str_getTime();
        float now = startTime - systemStartupTime;

        float min_deadline_distance = UINT8_MAX;
        uint8_t min_task = UINT8_MAX;
        for (uint8_t i = 0; i < N_SCHED_TASKS; i++) {
            TaskHandle_t handle = TaskHandles[i];
            eTaskState state = eTaskGetState(handle);
            if (state != eSuspended) continue;

            TaskEDF_t *const task = &TaskEDF[i];

            if (now > task->next_deadline) {
                delay(500);
                Serial.print("Task t");
                Serial.print(i+1);
                Serial.println(" missed its deadline!");
                Serial.print("Time = ");
                Serial.println(now, 10);
                Serial.print("Deadline = ");
                Serial.println(task->next_deadline, 10);
                Serial.println("###");

                xTimerStop(xOneShotTimer, 1000);
                OneShotTimerCallback(NULL);
            }
            float deadline_distance = task->next_deadline - now;
            if (deadline_distance < min_deadline_distance) {
                min_task = i;
                min_deadline_distance = deadline_distance;
            }
        }

        // set priority to tasks
        // vTaskPrioritySet(TaskHandles[previous_task], 0); // lower priority of current task
        // vTaskPrioritySet(TaskHandles[min_task], configMAX_PRIORITIES-2); // set priority of earliest task to maximum

        if (min_task < N_SCHED_TASKS && previous_task != min_task) {
            vTaskSuspend(TaskHandles[previous_task]);
            vTaskResume(TaskHandles[min_task]);
        }
        previous_task = min_task;

        float schedTime = str_getTime() - startTime;
        accSchedTime += schedTime;
        maxSchedTime = max(maxSchedTime, schedTime);
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(1));
    }
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char* pcTaskName) { 
    Serial.println(pcTaskName);
    for (;;);
}

void OneShotTimerCallback(TimerHandle_t xTimer) {
    TickType_t xTimeNow;
    xTimeNow = xTaskGetTickCount();

    str_trace();
    // Stop the kernel...
    vTaskSuspendAll();
    str_trace();
    vPortEndScheduler();

    //...and sent data to the host PC
    unsigned int i;
    for (i = 2; i < BUFF_SIZE; i++) {
		if (t[i] == 0) break;

        Serial.println("DAT");
        Serial.print((float)t[i]);
        for (uint8_t t = 0; t < N_TASKS; t++) {
            Serial.print(",");
            Serial.write(circ_buffers[t][i]);
        }
        Serial.println();
    }
    Serial.println("---");
    Serial.print("Samples: "); 
    Serial.println(i);

    Serial.print("System Startup Time: ");
    Serial.println(systemStartupTime);

    Serial.print("Max Trace Time: ");
    Serial.println(maxTraceTime, 10);
    Serial.print("Acc Trace Time: ");
    Serial.println(accTraceTime, 2);

    Serial.print("Max Sched Time: ");
    Serial.println(maxSchedTime, 10);
    Serial.print("Acc Sched Time: ");
    Serial.println(accSchedTime, 2);

    for (;;);
}

// str_getTime is a custom implementation of the time to debug data
float str_getTime(void) {
    // clear all;
    // SystemCoreClock=64000000%From Serial.println(SystemCoreClock);
    // configSYSTICK_CLOCK_HZ=SystemCoreClock
    // configTICK_RATE_HZ=100;
    // portNVIC_SYSTICK_LOAD_REG = ( configSYSTICK_CLOCK_HZ / configTICK_RATE_HZ );% - 1
    // tick2milliseconds=1/configTICK_RATE_HZ*1000/portNVIC_SYSTICK_LOAD_REG
    // vpa(tick2milliseconds,20)

    // #define portNVIC_SYSTICK_CURRENT_VALUE_REG    ( *( ( volatile uint32_t * ) 0xe000e018 ) )
    volatile uint32_t portNVIC_SYSTICK_CURRENT_VALUE_REG = (*((volatile uint32_t*)0xe000e018));

    // float t=(float)micros() / 1000.f;//ok
    // float t=millis();//not so precise
    float t = 0.0000015625 * ((640000 - portNVIC_SYSTICK_CURRENT_VALUE_REG) + xTaskGetTickCount() * 640000);
    //(float)(0.5e-3*((float)OCR1A*xTaskGetTickCount()+TCNT1));//Sent time in milliseconds!!!
    return t;
}

// str_compute(x) is only used to waste time without using delays
void str_compute(unsigned long milliseconds) {
    unsigned int i = 0;
    unsigned int imax = 0;
    imax = milliseconds * 3275;
    volatile float dummy = 1;
    for (i = 0; i < imax; i++) {
        dummy = dummy * dummy;
    }
}

// str_trace is a hook by the RTOS kernel used after a context-switch-in
void str_trace(void) {
    float startTime = str_getTime();

    circ_buffer_counter++;
    if (circ_buffer_counter >= BUFF_SIZE) {
        circ_buffer_counter = 0;
    }

    t[circ_buffer_counter] = str_getTime();  // sent time in milliseconds
    for (int i = 0; i < N_TASKS; i++) {
        circ_buffers[i][circ_buffer_counter] = '0' + eTaskGetState(TaskHandles[i]); 
    }

    float time = str_getTime() - startTime;
    accTraceTime += time;
    maxTraceTime = max(maxTraceTime, time);
}
