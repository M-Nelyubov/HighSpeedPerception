// Motor output pins. [L,L, R,R]  {lo, hi} gives forward rotation, {hi, lo} gives backward rotation
int pins[] = {D0, D1, D2, D3};
int Lpin = pins[1];
int Rpin = pins[3];

// 80 is the upper bound as this is the ideal cycle time for new data to come in from the camera
#define CYCLE_DUR_MS 80
#define STATIC_BREAKING_LOWER_BOUND 15

// 0-80 scale for duty cycle time as a portion of full time on.  
int dutyL = STATIC_BREAKING_LOWER_BOUND;
int dutyR = STATIC_BREAKING_LOWER_BOUND;


void setup() {
  for(int i=0;i<4;i++){
    pinMode(pins[i], OUTPUT);    // set all 4 pins to output
    digitalWrite(pins[i], LOW);  // initialize all 4 pins to off
  }
  Serial.begin(115200);
}


void loop() {
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
