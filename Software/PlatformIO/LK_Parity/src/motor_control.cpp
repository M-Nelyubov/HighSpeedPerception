#include <math.h>
#include "motor_control.hpp"

#define sq(x) ((x)*(x))

#define MAX_POW 80 // CYCLE_DUR_MS from motors.hpp, except importing is messy

int nonzero = 0;
int totalNegTheta = 0;
int totalPosTheta = 0;

int16_t mag(int u, int v){
  return u*u + v*v;
}

int16_t rule(int u, int v){
  // magnitude squared, for computational simplicity of not taking square roots on a board that says they're 0
  int m = mag(u,v);
  nonzero += m>0;
  return m >= OFIT * OFIT;
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

  // printf("U[0][0..9] = %d, %d, %d, %d, %d, %d, %d, %d, %d, %d\t", U[0], U[1], U[2], U[3], U[4], U[5], U[6], U[7], U[8], U[9]);
  printf("U[0][8..9] = %d, %d\t", U[8], U[9]);

  for(int r=0; r < IMAGE_ROWS; r++){
    for(int c=0; c < IMAGE_COLS; c++){
      int i = r * IMAGE_COLS + c;

      // contribute to the side's sum
      if(c < IMAGE_COLS/2) {
        leftCount++;
        leftSum+=mag(U[i],V[i]);
      } else {
        rightCount++;
        rightSum+=mag(U[i],V[i]);
      }       
    }
  }

  // int norm = (IMAGE_ROWS*IMAGE_COLS/2);

  float diff = (float)(leftSum - rightSum);
  float dTheta = atan(diff);
  int mag = (int) abs(dTheta);

  if(dTheta < 0){ // turn right
    printf("turning R ");
    totalNegTheta++;
    if(ctrl[0] < MAX_POW){ // if L < 100% duty
      ctrl[0] += mag;    // increase L
      printf("L++ ");
    } else {
      ctrl[1] -= mag;    // reduce R
      printf("R-- ");
    }
  }

  if(dTheta > 0){ // turn left
    printf("turning L ");
    totalPosTheta++;
    if(ctrl[1] < MAX_POW){ // R < 100% duty
      ctrl[1] += mag;    // increase R
      printf("R++ ");
    } else {
      ctrl[0] -= mag;    // reduce L
      printf("L-- ");
    }
  }

  // clip lower bound
  ctrl[0] = max(ctrl[0], -80);
  ctrl[1] = max(ctrl[1], -80);

  // for(int r=0; r < IMAGE_ROWS; r++){
  //   for(int c=0; c < IMAGE_COLS; c++){
  //     int i = r * IMAGE_COLS + c;
  //     if(c < IMAGE_COLS/2) {L_div+=sq(U[i] - L_u);} else {R_div += sq(U[i] - R_u);}                 // contribute to the side's sum
  //   }
  // }


  // print how many flow points are above the turning threshold
  printf("raw OF points L: %d\tR:%d\tnz:%d\tSUM L:%d\tR:%d\tdiff:%.6d\tdθ:%f\t", leftCount, rightCount, nonzero, leftSum, rightSum, (int) diff, dTheta); // Var L:%3.3f\tR:%3.3f\t  , sqrt(L_div/norm), sqrt(R_div/norm));

  printf("θ:[+%d][-%d]\t", totalNegTheta, totalPosTheta);

  // turn off the motor on the OTHER side to get the bot to turn away from that
  // with no flow, go full forward.  
  // The more flow, the more you slow down.  
  // At over 100 collisions, reverse.
  // ctrl[1] = 100 - leftCount;
  // ctrl[0] = 100 - rightCount;
}
