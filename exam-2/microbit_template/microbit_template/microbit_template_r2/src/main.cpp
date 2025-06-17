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

// methods
void Task1(void* pvParameters);
void Task2(void* pvParameters);
void Task3(void* pvParameters);
void Task4(void* pvParameters);
void Task5(void* pvParameters);
void Task9Scheduler(void* pvParameters);
void OneShotTimerCallback(TimerHandle_t xTimer);

void str_compute(unsigned long milliseconds);
void str_trace(void);
float str_getTime(void);

// edf
typedef struct {
    const char* name;
    const TaskFunction_t taskCode;
    const uint8_t deadline;
    const uint8_t period;
    TickType_t lastActivationTick;
    uint8_t remaining_deadline;
    TaskHandle_t handle;
} TaskEDF_t;

typedef struct {
    TaskEDF_t* edf;
} TaskDeadline_t;

TaskEDF_t TaskEDF[] = {
    {
        .name = "Task1",
        .taskCode = Task1,
        .deadline = 15,
        .period = 30,
    },
    {
        .name = "Task2",
        .taskCode = Task2,
        .deadline = 20,
        .period = 30,
    },
    {
        .name = "Task3",
        .taskCode = Task3,
        .deadline = 35,
        .period = 40,
    },
    {
        .name = "Task4",
        .taskCode = Task4,
        .deadline = 40,
        .period = 50,
    },
    {
        .name = "Task5",
        .taskCode = Task5,
        .deadline = 50,
        .period = 50,
    },
};
const uint8_t N_SCHED_TASKS = sizeof(TaskEDF) / sizeof(TaskEDF_t);
const uint8_t N_TASKS = N_SCHED_TASKS + 1;

TaskDeadline_t TaskDeadlines[N_SCHED_TASKS] = {};
TaskHandle_t TaskHandles[N_TASKS] = {};

// circular buffer for debugging
const size_t BUFF_SIZE = TSTOP * 5;
float t[BUFF_SIZE] = {};
char circ_buffers[N_TASKS][BUFF_SIZE] = {};
unsigned int circ_buffer_counter = 0;

// task handlers
float systemStartupTime;
float maxTraceTime = INT32_MIN;
float accTraceTime = 0;
float maxSchedTime = INT32_MIN;
float accSchedTime = 0;

// timer handlers
TimerHandle_t xPeriodicTimer, xOneShotTimer;
BaseType_t xPeriodicTimerStarted, xOneShotStarted;

void setup()  // put your setup code here, to run once:
{
    pinMode(LED1, OUTPUT);
    pinMode(COL1, OUTPUT);
    digitalWrite(COL1, LOW);
    Serial.begin(115200);

    for (uint8_t i = 0; i < N_SCHED_TASKS; i++) {
        TaskEDF_t* task = &TaskEDF[i];
        xTaskCreate(task->taskCode, task->name, configMINIMAL_STACK_SIZE, NULL, 1, &TaskHandles[i]);
        task->handle = TaskHandles[i];
        task->remaining_deadline = task->deadline;
        TaskDeadlines[i] = {.edf = task};
    }
    xTaskCreate(Task9Scheduler, "Task9", configMINIMAL_STACK_SIZE, NULL, configMAX_PRIORITIES - 1, &TaskHandles[N_SCHED_TASKS]);

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

    TickType_t xLastWakeTime = 0;

    for (;;) {
        str_compute(2);

        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(30));
    }
}

/// C = 4 ms
/// D = 20 ms
/// P = 30 ms
void Task2(void* pvParameters) {
    (void)pvParameters;

    TickType_t xLastWakeTime = 0;

    for (;;) {
        str_compute(4);

        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(30));
    }
}

/// C = 10 ms
/// D = 35 ms
/// P = 40 ms
void Task3(void* pvParameters) {
    (void)pvParameters;

    TickType_t xLastWakeTime = 0;

    for (;;) {
        str_compute(10);

        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(40));
    }
}

/// C = 21 ms
/// D = 40 ms
/// P = 50 ms
void Task4(void* pvParameters) {
    (void)pvParameters;

    TickType_t xLastWakeTime = 0;

    for (;;) {
        str_compute(21);

        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(50));
    }
}

/// C = 5 ms
/// D = 50 ms
/// P = 50 ms
void Task5(void* pvParameters) {
    (void)pvParameters;

    TickType_t xLastWakeTime = 0;

    for (;;) {
        str_compute(5);

        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(50));
    }
}

int cmpDeadlines(const void* a, const void* b) {
    const auto arg1 = static_cast<const TaskDeadline_t*>(a)->edf->remaining_deadline;
    const auto arg2 = static_cast<const TaskDeadline_t*>(b)->edf->remaining_deadline;
    if (arg1 < arg2) {
        return -1;
    } else if (arg1 > arg2) {
        return 1;
    }
    return 0;
}

void Task9Scheduler(void* arg) {
    (void)arg;

    for (;;) {
        float startTime = str_getTime();

        TickType_t now = xTaskGetTickCount();
        
        for (uint8_t i = 0; i < N_SCHED_TASKS; i++) {
            TaskEDF_t* edf = TaskDeadlines[i].edf;
            eTaskState state = eTaskGetState(edf->handle);
            if (now - edf->lastActivationTick >= edf->period) {
                edf->lastActivationTick = now;
                edf->remaining_deadline = edf->deadline;
            }
            if (state == eReady || state == eRunning) {
                if (edf->remaining_deadline > 0) edf->remaining_deadline -= 1;
                else if (false) {
                    // Si s'activa la branca es perd el deadline immediatament
                    // Si es desactiva, podem veure que no és el cas amb el graf

                    str_compute(500);
                    // deadline missed
                    Serial.print(edf->name);
                    Serial.println(" missed deadline");
                    Serial.print("Deadline: ");
                    Serial.println((uint32_t)edf->lastActivationTick + edf->deadline);
                    Serial.print("Now: ");
                    Serial.println((uint32_t)now);
                    Serial.println("###");

                    xTimerStop(xOneShotTimer, 0);
                    OneShotTimerCallback(NULL);

                    for (;;);
                }
            }
        }

        std::qsort(TaskDeadlines, N_SCHED_TASKS, sizeof(TaskDeadline_t), cmpDeadlines);
        
        for (uint8_t i = 0; i < N_SCHED_TASKS; i++) {
            vTaskPrioritySet(TaskDeadlines[i].edf->handle, configMAX_PRIORITIES - 2 - i);
        }

        float time = str_getTime() - startTime;
        accSchedTime += time;
        maxSchedTime = max(maxSchedTime, time);

        vTaskSuspend(NULL);
    }
}

void vApplicationTickHook(void) {
    xTaskResumeFromISR(TaskHandles[N_SCHED_TASKS]);
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char* pcTaskName) {
    Serial.println(pcTaskName);
    for (;;);
}

void OneShotTimerCallback(TimerHandle_t xTimer) {
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
    for (int i = 0; i < N_SCHED_TASKS; i++) {
        circ_buffers[i][circ_buffer_counter] = '0' + eTaskGetState(TaskHandles[i]);
    }

    // workaround to get graph to work properly
    circ_buffers[N_SCHED_TASKS][circ_buffer_counter] = eTaskGetState(TaskHandles[N_SCHED_TASKS]) == eSuspended ? '2' : '0';

    float time = str_getTime() - startTime;
    accTraceTime += time;
    maxTraceTime = max(maxTraceTime, time);
}
