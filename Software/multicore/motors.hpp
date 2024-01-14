#include <Arduino.h>

// 80 is the upper bound as this is the ideal cycle time for new data to come in from the camera
#define CYCLE_DUR_MS 80
#define STATIC_BREAKING_LOWER_BOUND 15

// void motor_setup();

/**
  Runs the motors in forward configuration for the specified number of milliseconds for each motor
  default behavior without parameters is going to run the motor for the entire duration cycle at full power
*/
// void run_motor_cycle(int dutyL = CYCLE_DUR_MS, int dutyR = CYCLE_DUR_MS);
