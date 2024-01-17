/*********
  Rui Santos
  Complete project details at http://randomnerdtutorials.com  
*********/
#define CYCLE_DUR_MS 80

TaskHandle_t motorTask;
int ticks = 0;
int duties[] = {0,0};


void setup() {
  Serial.begin(115200);
  Serial.println("Starting up");
  motorSetup();
}

void motorSetup(){
  int motor_pins[4] = {D0, D1, D2, D3};
  for(int i=0;i<4;i++){
    pinMode(motor_pins[i], OUTPUT);    // set all 4 pins to output
    digitalWrite(motor_pins[i], LOW);  // initialize all 4 pins to off
  }

  setPower(0,0);  // initial starting values of not powered
}

//core0motorControl: blinks an LED every 1000 ms
void core0motorControl( void * pvParameters ){
  // Serial.println("Starting core 0 task - motor control cycle");
  int *duties = (int *) pvParameters;
  int dL = duties[0];
  int dR = duties[1];
  Serial.printf("motorTask (t=%d) running on core %d [ticks = %d] Pow: [%x,%x] (&d:%x) \n", millis(), xPortGetCoreID(), ticks, dL, dR, duties);
  powerCycle(dL, dR);
  vTaskDelete(NULL);
}


void setPower(int dutyL, int dutyR){
  duties[0] = dutyL;
  duties[1] = dutyR;
  
  // https://randomnerdtutorials.com/esp32-dual-core-arduino-ide/
  xTaskCreatePinnedToCore(
    core0motorControl,   /* Task function. */
    "motorTask",     /* name of task. */
    10000,       /* Stack size of task */
    &duties,        /* parameter of the task */
    1,           /* priority of the task */
    &motorTask,      /* Task handle to keep track of created task */
    0);          /* pin task to core 0 */

}

void loop() {
  for(int i =0; i<100; i++){
    setPower(i, 100-i);
    delay(80);
  }
}


void powerCycle(int dL, int dR){
  auto Lpin = D1;
  auto Rpin = D3;

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
