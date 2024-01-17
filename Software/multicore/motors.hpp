#include <Arduino.h>

// 80 is the upper bound as this is the ideal cycle time for new data to come in from the camera
#define CYCLE_DUR_MS 80
#define STATIC_BREAKING_LOWER_BOUND 15


/**
  Run on startup to configure motor pins
*/
void motorSetup();

/**
  Sets the duty cycle for the left and right motors for the next CYCLE_DUR_MS milliseconds
*/
void setPower(int dutyL, int dutyR);
