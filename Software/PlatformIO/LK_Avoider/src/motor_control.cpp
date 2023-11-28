#include "motor_control.hpp"

void motorControl(int16_t U[IMAGE_ROWS * IMAGE_COLS], int16_t V[IMAGE_ROWS * IMAGE_COLS], int ctrl[2]){
  // Parameters: 
  //   U optical flow component, 
  //   V optical flow component, 
  //   motor control output signals (0 Left, 1 Right)
  int LEFT = 0;
  int RIGHT = 1;
  
  // initially, keep the signals on
  ctrl[LEFT] = 1;
  ctrl[RIGHT] = 1;

  // count flow in each half of the screen
  int leftSum = 0;
  int rightSum = 0;

  for(int r=0; r < IMAGE_ROWS; r++){
    for(int c=0; c < IMAGE_COLS; c++){
      int i = r * IMAGE_COLS + c;
      int mag = U[i]*U[i] + V[i]*V[i];                                // magnitude squared, for computational simplicity
      if(mag >= OPTICAL_FLOW_INTENSITY_THRESHOLD * OPTICAL_FLOW_INTENSITY_THRESHOLD){   // compare against the threshold
          if(c < IMAGE_COLS/2) {leftSum++;} else {rightSum++;}                           // contribute to the side's sum
      }
    }
  }

  // print how many flow points are above the turning threshold
  // Serial.printf("L: %d\tR:%d\t", leftSum, rightSum);

  // turn off the motor on the OTHER side to get the bot to turn away from that
  if(leftSum > OPTICAL_FLOW_QUANTITY_THRESHOLD) ctrl[RIGHT] = 0;
  if(rightSum > OPTICAL_FLOW_QUANTITY_THRESHOLD) ctrl[LEFT] = 0;
}
