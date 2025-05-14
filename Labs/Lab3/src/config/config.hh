#ifndef CONFIG_HH
#define CONFIG_HH

#include <Arduino.h>

/* NOTE: Types like uint8_t or similar are an alias for unsigned int or similar. */

/*******************/
/* BOARD VARIABLES */
/*******************/

// Controls the motor direction
const uint8_t DIR_A = 12;

// Controls the speed of the motor
const uint8_t PWM_A = 3;

// Hall effect sensors
const uint8_t HALL_A = 18;
const uint8_t HALL_B = 19;

/******************/
/* TASK VARIABLES */
/******************/

uint8_t Task1RunningPin = 0;

// This must be a signed integer since the motor
// must be able to rotate in both directions
int16_t Task1HallCounter = 0;

/*******************/
/* MOTOR VARIABLES */
/*******************/

// Make sure to the speed is always between 75 and 254
uint8_t motor_speed = 120;

bool is_motor_clockwise = true;

int8_t motor_angle = 0;

// Target angle for each movement
int8_t reference_angle = 90;

// Motor angle calculation variables
const int16_t ppr = 7 * 2 * 2 * 50;  // 1400
const float hall_delta = 360.0f / ppr;

/***********************/
/* EXECUTION VARIABLES */
/***********************/

// After the amount of time specified in executionTime (ms),
// the program will go into a complete halt.
const uint16_t execution_time = 30000;

#endif
