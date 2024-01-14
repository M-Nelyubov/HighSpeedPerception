/*********
  Rui Santos
  Complete project details at http://randomnerdtutorials.com  
*********/

#include "motors.hpp"

Motors motors;

void setup() {
  Serial.begin(115200);
  Serial.print("setup() running on core ");
  Serial.println(xPortGetCoreID());

  motors.setup();
  motors.start();  
}

void loop() {
  Serial.printf("loop() running on core %d, state: %d\n", xPortGetCoreID(), motors.state);
  delay(1000);
}


