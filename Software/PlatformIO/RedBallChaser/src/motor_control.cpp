#include <math.h>
#include "motor_control.hpp"

#define sq(x) (x)*(x)

int nonzero = 0;

int16_t rule(int u, int v){
  // magnitude squared, for computational simplicity of not taking square roots on a board that says they're 0
  int mag = u*u + v*v;
  nonzero += mag>0;
  return mag >= OFIT * OFIT;
}

void motorControl(int16_t U[IMAGE_ROWS * IMAGE_COLS], int16_t V[IMAGE_ROWS * IMAGE_COLS], int ctrl[2]){
  // Parameters: 
  //   U optical flow component, 
  //   V optical flow component, 
  //   motor control output signals (0 Left, 1 Right)
  
  // count flow in each half of the screen
  int leftCount = 0;
  int rightCount = 0;
  int leftSum = 0;
  int rightSum = 0;
  int points = 0;
  nonzero = 0;

  for(int r=0; r < IMAGE_ROWS; r++){
    for(int c=0; c < IMAGE_COLS; c++){
      int i = r * IMAGE_COLS + c;
      points++;
      int16_t rl = rule(U[i], V[i]);
      if(rl){
          if(c < IMAGE_COLS/2) {leftCount++;} else {rightCount++;}                           // contribute to the side's sum
      }
      if(c < IMAGE_COLS/2) {leftSum+=(U[i]);} else {rightSum+=(U[i]);}                 // contribute to the side's sum
    }
  }

  int norm = (IMAGE_ROWS*IMAGE_COLS/2);

  int L_u = leftSum / norm;
  int R_u = rightSum / norm;

  int L_div = 0;
  int R_div = 0;

  for(int r=0; r < IMAGE_ROWS; r++){
    for(int c=0; c < IMAGE_COLS; c++){
      int i = r * IMAGE_COLS + c;
      if(c < IMAGE_COLS/2) {L_div+=sq(U[i] - L_u);} else {R_div += sq(U[i] - R_u);}                 // contribute to the side's sum
    }
  }


  // print how many flow points are above the turning threshold
  printf("raw OF points L: %d\tR:%d\tnz:%d\tMEAN L:%d\tR:%d\tVar L:%3.3f\tR:%3.3f\t", leftCount, rightCount, nonzero, L_u, R_u, sqrt(L_div/norm), sqrt(R_div/norm));

  // turn off the motor on the OTHER side to get the bot to turn away from that
  // with no flow, go full forward.  
  // The more flow, the more you slow down.  
  // At over 100 collisions, reverse.
  ctrl[1] = 100 - leftCount;
  ctrl[0] = 100 - rightCount;
}

void motorControl(float x, float mag, int ctrl[2]){
  // When not detecting a target, don't move.
  if(mag < 1000){
    ctrl[0] = ctrl[1] = 0;
    return;
  }

  if(abs(x) > .5){
  // steering at the outer edges (|x| > .5)
  // direct proportional control adjustment based on position
  // for x = 1, the ball is to the right, so turn right with ctrl R ++
  // for x = 1, the ball is to the left, so turn left with ctrl L ++
  ctrl[0] -= (int) (70.0f * x);
  ctrl[1] += (int) (70.0f * x);
  } else {
    // at the inner/central midpoint, aim more directly to just go full speed ahead 
    ctrl[0] = 70 - (int)(10.0f * x);
    ctrl[1] = 50 + (int)(10.0f * x);
  }
}
