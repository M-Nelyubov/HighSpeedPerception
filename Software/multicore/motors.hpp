#include <Arduino.h>

// 80 is the upper bound as this is the ideal cycle time for new data to come in from the camera
#define CYCLE_DUR_MS 80
#define STATIC_BREAKING_LOWER_BOUND 15

// Pinouts for forward and reverse signals on motors
#define L_RVS D0
#define L_FWD D1
#define R_RVS D2
#define R_FWD D3

#define PRIMARY_CORE 1
#define SECONDARY_TASK_CORE 0

/**
  Run on startup to configure motor pins
*/
void motorSetup();

/**
  Sets the duty cycle for the left and right motors for the next CYCLE_DUR_MS milliseconds
*/
void setPower(int dutyL, int dutyR);
