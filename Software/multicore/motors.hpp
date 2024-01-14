#include <Arduino.h>

// 80 is the upper bound as this is the ideal cycle time for new data to come in from the camera
#define CYCLE_DUR_MS 80
#define STATIC_BREAKING_LOWER_BOUND 15

class Motors {
  private:
    TaskHandle_t motor_task;

    // Motor output pins. [L,L, R,R]  {lo, hi} gives forward rotation, {hi, lo} gives backward rotation
    int motor_pins[4] = {D0, D1, D2, D3};
    int Lpin = motor_pins[1];
    int Rpin = motor_pins[3];

  public:
    int dutyL = 100;
    int dutyR = 100;

    int state = 0;

    void setup();
    void start();
    static void cycleMotors(void * p);

};
