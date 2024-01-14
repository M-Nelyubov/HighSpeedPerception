/*********
  Rui Santos
  Complete project details at http://randomnerdtutorials.com  
*********/
#define CYCLE_DUR_MS 80

TaskHandle_t motorTask;
int ticks = 0;

void powerCycle(int dL = 100, int dR = 100);

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

  int duties[] = {50, 50};

  // https://randomnerdtutorials.com/esp32-dual-core-arduino-ide/
  xTaskCreatePinnedToCore(
    core0motorControl,  /* Task function. */
    "motorTask",        /* name of task. */
    10000,              /* Stack size of task */
    &duties,            /* parameter of the task */
    1,                  /* priority of the task */
    &motorTask,         /* Task handle to keep track of created task */
    0);                 /* pin task to core 0 */
}

//core0motorControl: blinks an LED every 1000 ms
void core0motorControl( void * pvParameters ){
    int *duties = (int *) pvParameters;
    int dL = duties[0];
    int dR = duties[1];
    Serial.printf("motorTask (t=%d) running on core %d [ticks = %d]\n", millis(), xPortGetCoreID(), ticks);
    powerCycle(dL, dR);
    vTaskDelete(motorTask);
}


void loop() {}


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
    ticks++;
  }
}

