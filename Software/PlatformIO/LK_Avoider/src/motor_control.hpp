#ifndef motorControl_hpp
#define motorControl_hpp

#include "camera_config.hpp"

#include <iostream>
#include <stdio.h>
using namespace std;

#define OPTICAL_FLOW_INTENSITY_THRESHOLD 5     // the magnitude of flow after which point, it counts toward taking evasive behavior
#define OPTICAL_FLOW_QUANTITY_THRESHOLD 15     // the number of flow points above the threshold, requiring at least this many to take evasive action


/**
 * U - the U (-1 Left, 0 - stationary, +1 Right) optical flow values of all pixels
 * V - the V (-1 Down, 0 - stationary, +1 Up)    optical flow values of all pixels
 * ctrl - the control signals to send to the motors. 0 - off, 1 - forward
*/
void motorControl(int16_t U[IMAGE_ROWS * IMAGE_COLS], int16_t V[IMAGE_ROWS * IMAGE_COLS], int ctrl[2]);


#endif
