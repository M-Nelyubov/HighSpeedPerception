/*********
  Rui Santos
  Complete project details at http://randomnerdtutorials.com  
*********/
#include "motors.hpp"


void setup() {
  Serial.begin(115200);
  Serial.println("Starting up");
  motorSetup();
}

void loop() {
  for(int i =0; i<100; i++){
    setPower(i, 100-i);
    delay(80);
  }
}

