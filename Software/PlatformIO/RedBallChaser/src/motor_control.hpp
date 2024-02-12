#ifndef motorControl_hpp
#define motorControl_hpp

#include "camera_config.hpp"

#include <iostream>
#include <stdio.h>
using namespace std;

#define OFIT 10    // optical flow intensity threshold - the magnitude of flow after which point, it counts toward taking evasive behavior
#define OFQT 50    // optical flow quantity threshold - the number of flow points above the threshold, requiring at least this many to take evasive action


/**
 * U - the U (-1 Left, 0 - stationary, +1 Right) optical flow values of all pixels
 * V - the V (-1 Down, 0 - stationary, +1 Up)    optical flow values of all pixels
 * ctrl - the control signals to send to the motors. 
 *          Entry at index: 0 - left motor.  1 - right motor.
 *          Values: 0 - off. 100 - fully forward.  -100 - fully backward.
 */
void motorControl(int16_t U[IMAGE_ROWS * IMAGE_COLS], 
                  int16_t V[IMAGE_ROWS * IMAGE_COLS], 
                  int ctrl[2]);

/**
 * Motor control based on a fixed reference value in the pixel space that the vehicle should turn toward
 * input x - the position to turn to, based on pixel coordinates with x=0 being center, x=-1 being far left, and x=1 being far right.
 * ctrl - the control signals to send to the motors. 
 *          Entry at index: 0 - left motor.  1 - right motor.
 *          Values: 0 - off. 100 - fully forward.  -100 - fully backward.
*/
void motorControl(float x, int ctrl[2]);

/**
 * The rule for whether or not a flow point has breached the critical threshold for motion to be counted torward the limit at which a motor should stop
*/
int16_t rule(int u, int v);


#endif
