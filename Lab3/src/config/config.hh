#ifndef CONFIG_HH
#define CONFIG_HH

/* NOTE: Types like uint8_t or similar are an alias for unsigned int or similar. */

/*******************/
/* BOARD VARIABLES */
/*******************/

// Controls the motor direction
const unsigned int DIR_A = 12;

// Controls the speed of the motor
const unsigned int PWM_A = 3;

// Hall effect sensors
const unsigned int HALL_A = 18;
const unsigned int HALL_B = 19;

/******************/
/* TASK VARIABLES */
/******************/

unsigned int Task2RunningPin = 0;

// This must be a signed integer since the motor
// must be able to rotate in both directions
int Task2HallCounter = 0;

/*******************/
/* MOTOR VARIABLES */
/*******************/

// Make sure to the speed is always between 75 and 254
unsigned int motor_speed = 120;

unsigned char is_motor_clockwise = true;

int motor_angle = 0;

// Target angle for each movement
const unsigned int movement_angle = 90;

// Motor angle calculation variables
const unsigned int ppr = 7 * 2 * 2 * 50;  // 1400
const float hall_delta = 360.0f / ppr;

/***********************/
/* EXECUTION VARIABLES */
/***********************/

// After the amount of time specified in executionTime (ms),
// the program will go into a complete halt.
const unsigned int execution_time = 30000;

#endif
