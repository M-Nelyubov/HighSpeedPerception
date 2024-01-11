#include "motor_control.hpp"

int16_t rule(int u, int v){
  // magnitude squared, for computational simplicity of not taking square roots on a board that says they're 0
  int mag = u*u + v*v;
  return mag >= OFIT * OFIT;
}

void motorControl(int16_t U[IMAGE_ROWS * IMAGE_COLS], int16_t V[IMAGE_ROWS * IMAGE_COLS], int ctrl[2]){
  // Parameters: 
  //   U optical flow component, 
  //   V optical flow component, 
  //   motor control output signals (0 Left, 1 Right)
  
  // count flow in each half of the screen
  int leftSum = 0;
  int rightSum = 0;

  for(int r=0; r < IMAGE_ROWS; r++){
    for(int c=0; c < IMAGE_COLS; c++){
      int i = r * IMAGE_COLS + c;
      if(rule(U[i], V[i])){
          if(c < IMAGE_COLS/2) {leftSum++;} else {rightSum++;}                           // contribute to the side's sum
      }
    }
  }

  // print how many flow points are above the turning threshold
  // Serial.printf("L: %d\tR:%d\t", leftSum, rightSum);

  // turn off the motor on the OTHER side to get the bot to turn away from that
  // with no flow, go full forward.  
  // The more flow, the more you slow down.  
  // At over 100 collisions, reverse.
  ctrl[1] = 100 - leftSum;
  ctrl[0] = 100 - rightSum;
}
