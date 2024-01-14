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
  Serial.print("loop() running on core ");
  Serial.println(xPortGetCoreID());
  delay(1000);
}


