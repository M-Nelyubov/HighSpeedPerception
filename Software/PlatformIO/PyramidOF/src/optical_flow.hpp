#ifndef opticalFlow_hpp
#define opticalFlow_hpp

#include <iostream>
#include <stdio.h>
using namespace std;

// Image Dimensions
#define IMAGE_ROWS 96
#define IMAGE_COLS 96

#define OF_WINDOW 4   // optical flow coherence constraint window size (N x N)
#define OF_FRAME_SKIPS 4 // optical flow is only calculated once per this many frames in each dimension

#define PYRAMID_DEPTH 0

/**
 * p_frame - previous image frame, greyscale byte grid
 * n_frame - next image frame, same
 * u_frame - the U optical flow values of all pixels
 * v_frame - the V optical flow values of all pixels
*/
void computeFlow(
    uint8_t p_frame[IMAGE_ROWS * IMAGE_COLS], 
    uint8_t n_frame[IMAGE_ROWS * IMAGE_COLS], 
    int16_t u_frame[IMAGE_ROWS * IMAGE_COLS], 
    int16_t v_frame[IMAGE_ROWS * IMAGE_COLS] 
    );


#endif
