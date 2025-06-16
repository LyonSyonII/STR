# <span style="color: aqua;">Porting FreeRTOS to microbit from scratch</span> 
---
---
## <span style="color: lightskyblue;">Create a BBC micro:bit V2 project in PlatformIO</span>
---

Open VSCode and create a new PlatformIO project:  
- give it a name  
- select ```BBC micro:bit V2 board```  
- select ```Arduino``` framework  
- save it wherever you want  

Let's create a first project:  
- check this code:

```c
#include <Arduino.h>
const int LED1= 21;
const int COL1= 4;
int led1State=LOW;

void setup() // put your setup code here, to run once:
{
    pinMode(LED1,OUTPUT);
    pinMode(COL1, OUTPUT);
    digitalWrite(COL1,LOW);
    Serial.begin(115200);
}

void loop() // put your main code here, to run repeatedly:
{
    led1State ^= 1;
    digitalWrite(LED1,led1State);
    Serial.print("t=");
    Serial.println(millis());
    delay(500);
}
```

- add ```monitor_speed=115200``` to ```platformio.ini```  
- click ```F1``` and select ```PlatformIO: Upload and Monitor``` 
- the output terminal should look like  
```c
t=501
t=1001
t=1501
...
```
- also the top-left led of the microbit should be blinking  

## <span style="color: lightskyblue;">Download FreeRTOS</span>
---

- Download FreeRTOS from https://www.freertos.org  
On the top-right of the webpage, there is a Download button witch points to:  
https://github.com/FreeRTOS/FreeRTOS-LTS/releases/download/202406.01-LTS/FreeRTOSv202406.01-LTS.zip  

- Unzip the downloaded file  

- Copy ```...\FreeRTOSv202406.01-LTS\FreeRTOS\FreeRTOS-Kernel``` into the Platformio project, into the ```lib``` folder  

- Remove everything in ```\portable``` folder except ```\lib\FreeRTOS-Kernel\portable\GCC\ARM_CM4F``` i.e. the ```port.c``` and ```portmacro.h``` files.
Also keep the ```\portable\MemMag``` folder i.e. the memory managers 

- From the ```\lib\FreeRTOS-Kernel\examples\template_configuration``` copy the ```FreeRTOSConfig.h``` into the ```\lib\FreeRTOS-Kernel\include``` folder

## <span style="color: lightskyblue;">Make FreeRTOS compile</span>
---

- Add paths to PlatformIO by modifying ```platformio.ini``` as follows:
```
build_flags = 
    -Ilib/FreeRTOS-Kernel
    -Ilib/FreeRTOS-Kernel/include
    -Ilib/FreeRTOS-Kernel/portable/GCC/ARM_CM4F
    -Ilib/FreeRTOS-Kernel/portable/MemMang
```

- Add ```#include "FreeRTOS.h"``` and ```#include "task.h"``` in the ```main.cpp```

- Add the task to the ```setup()``` and start the scheduler

- Create a task

- The code should look like:
```c
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
```

- The first compilation error is:
```tasks.c:(.text.vTaskSwitchContext+0x3e): undefined reference to `vApplicationStackOverflowHook'```
To fix it, add the following code at the end of the ```main.cpp```:

```c
void vApplicationStackOverflowHook( TaskHandle_t xTask, char * pcTaskName )
{
  Serial.println(pcTaskName);
}
```

- There are <span style="color: orangered;">mandatory</span> changes into the ```FreeRTOSConfig.h``` file:
    - Above all, comment out the code above line 409 as follows
    ```c
    #define configASSERT( x )         \
    if( ( x ) == 0 )              \
    {                             \
        /*STR taskDISABLE_INTERRUPTS(); */\
        /*STR for( ; ; )                */\
        ;                         \
    }
    ```

    - Also, comment out line 54 and add the following code next to it:
    ```c
  //STR
  // #define configCPU_CLOCK_HZ    ( ( unsigned long ) 20000000 )
  extern uint32_t SystemCoreClock;
  #define configCPU_CLOCK_HZ   (SystemCoreClock)
  #define xPortSysTickHandler     SysTick_Handler
  #define vPortSVCHandler         SVC_Handler
  #define xPortPendSVHandler      PendSV_Handler
  //STR
    ```

    - Finally, line 85 should be:
    ```c
    #define configTICK_RATE_HZ                         1000//STR 100 
    ```
## <span style="color: lightskyblue;">Debugging</span>
<img src="https://upload.wikimedia.org/wikipedia/commons/2/21/Emojione_1F41E.svg" alt="MarineGEO circle logo" style="height: 100px; width:100px;"/>

- Click the debug button, it is located in the left bar of VSCode, the fourth button from the top. 
- After compiling and uploading it is possible to run the debug, stop it, reset, etc. 
- Also it is possible to add vars for debugging.
- Moreover, clicking on the left of any line number, a new breakpoint can be selected.

## <span style="color: lightskyblue;">Files included</span>

+ The file ```FreeRTOSv202406.01-LTS.zip``` is the file directly downloaded from the FreeRTOS webpage.
+ The ```microbit_template_r1``` is the first approach (developed during the class) with one task and FreeRTOS compiling as explained above. 
+ The ```microbit_template_r2``` includes ```str_compute(·)```, ```str_getTime()```, and the ```traceTASK_SWITCHED_IN str_trace()```. To start the *Exam 2- EDF on top of FreeRTOS*, use this one!!!