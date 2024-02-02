#include "motors.hpp"

TaskHandle_t motorTask;
int ticks = 0;    // number of times that the motor has been set high
int duties[] = {0,0}; // duty cycle global variable storage spot for transferability between cores

/**
  Wrapper for powerCycle to extract parameters from void *
*/
void core0motorControl( void * pvParameters );

/**
  Core Implementation of translating duty cycle inputs to power outputs to control pins
*/
void powerCycle(int dL, int dR);

void motorSetup(){
  int motor_pins[4] = {L_RVS, L_FWD, R_RVS, R_FWD};
  for(int i=0;i<4;i++){
    pinMode(motor_pins[i], OUTPUT);    // set all 4 pins to output
    digitalWrite(motor_pins[i], LOW);  // initialize all 4 pins to off
  }

  setPower(0,0);  // initial starting values of not powered
}

void setPower(int dutyL, int dutyR){
  // executes on primary core
  duties[0] = dutyL;
  duties[1] = dutyR;
  
  // https://randomnerdtutorials.com/esp32-dual-core-arduino-ide/
  xTaskCreatePinnedToCore(
    core0motorControl,    /* Task function. */
    "motorTask",          /* name of task. */
    10000,                /* Stack size of task */
    &duties,              /* parameter of the task */
    1,                    /* priority of the task */
    &motorTask,           /* Task handle to keep track of created task */
    SECONDARY_TASK_CORE); /* pin task to core 0 */

}

void core0motorControl( void * pvParameters ){
  // executes motor control task on secondary core to avoid scheduling problems with the main tasks on primary core
  int *duties = (int *) pvParameters;
  int dL = duties[0];
  int dR = duties[1];
  // Serial.printf("motorTask (t=%d) running on core %d [ticks = %d] Pow: [%d,%d]\n", millis(), xPortGetCoreID(), ticks, dL, dR);
  powerCycle(dL, dR);
  vTaskDelete(NULL);
}

void powerCycle(int dL, int dR){
  auto Lpin = L_FWD;
  auto Rpin = R_FWD;

  // parameters aren't used for anything, but are necessary for successful compilation
  // Serial.printf("cycleMotors running on core %d. t: %d\n", xPortGetCoreID(), millis());
  int t1 = millis();
  int dt = CYCLE_DUR_MS;  // 80ms cycle time
  int t2 = t1+dt;         // stop time

  // when to stop powering the motor
  int stopL = t1 + (dt * dL) / CYCLE_DUR_MS;
  int stopR = t1 + (dt * dR) / CYCLE_DUR_MS;

  // loop for the duration of the cycle
  for(int t = millis(); t<t2; t = millis()){
    digitalWrite(Lpin, !!(stopL > t));  // power the motor if the stop time for that motor hasn't passed yet
    digitalWrite(Rpin, !!(stopR > t));  // power the motor if the stop time for that motor hasn't passed yet
    delay(1);             // 1ms pause as this is the lowest coherent unit of time measurement for the board
    ticks += !!(stopL > t) + !!(stopR > t);
  }
}
