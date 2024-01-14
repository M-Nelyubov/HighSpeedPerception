#include "motors.hpp"


class Motors {
  private:
    TaskHandle_t motor_task;

    // Motor output pins. [L,L, R,R]  {lo, hi} gives forward rotation, {hi, lo} gives backward rotation
    int motor_pins[] = {D0, D1, D2, D3};
    int Lpin = motor_pins[1];
    int Rpin = motor_pins[3];


    void cycleMotors(void * parameters){ 
      // parameters aren't used for anything, but are necessary for successful compilation
      // Serial.printf("cycleMotors running on core %d. t: %d\n", xPortGetCoreID(), millis());
      int t1 = millis();
      int dt = CYCLE_DUR_MS;  // 80ms cycle time
      int t2 = t1+dt;         // stop time

      // when to stop powering the motor
      int stopL = t1 + (dt * dutyL) / CYCLE_DUR_MS;
      int stopR = t1 + (dt * dutyR) / CYCLE_DUR_MS;

      // loop for the duration of the cycle
      for(int t = millis(); t<t2; t = millis()){
        digitalWrite(Lpin, !!(stopL > t));  // power the motor if the stop time for that motor hasn't passed yet
        digitalWrite(Rpin, !!(stopR > t));  // power the motor if the stop time for that motor hasn't passed yet
        delay(1);             // 1ms pause as this is the lowest coherent unit of time measurement for the board
      }
    }

  public:
    int dutyL = 100;
    int dutyR = 100;

    Motors setup() {
      for(int i=0;i<4;i++){
        pinMode(motor_pins[i], OUTPUT);    // set all 4 pins to output
        digitalWrite(motor_pins[i], LOW);  // initialize all 4 pins to off
      }

      return this;
    }

    Motors start() {
      // spins up a thread on the the secondary core to run the motors at a given frequency

      // https://randomnerdtutorials.com/esp32-dual-core-arduino-ide/
      xTaskCreatePinnedToCore(
        cycleMotors,  /* Function to implement the task */
        "motor_task", /* Name of the task */
        10000,        /* Stack size in words - kept default */
        NULL,         /* Task input parameter */
        20,           /* Priority of the task */
        &motor_task,  /* Task handle. */
        0             /* Core where the task should run */
      );
  }
}
