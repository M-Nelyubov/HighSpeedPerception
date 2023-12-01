/*
  Blink

  Turns an LED on for one second, then off for one second, repeatedly.

  Most Arduinos have an on-board LED you can control. On the UNO, MEGA and ZERO
  it is attached to digital pin 13, on MKR1000 on pin 6. LED_BUILTIN is set to
  the correct LED pin independent of which board is used.
  If you want to know what pin the on-board LED is connected to on your Arduino
  model, check the Technical Specs of your board at:
  https://www.arduino.cc/en/Main/Products

  modified 8 May 2014
  by Scott Fitzgerald
  modified 2 Sep 2016
  by Arturo Guadalupi
  modified 8 Sep 2016
  by Colby Newman

  This example code is in the public domain.

  https://www.arduino.cc/en/Tutorial/BuiltInExamples/Blink
*/

int pins[] = {D0, D1, D2, D3}; //{1, 2, 3, 4};
// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  for(int i=0;i<4;i++){
    pinMode(pins[i], OUTPUT);
    digitalWrite(pins[i], LOW);
  }
  Serial.begin(115200);
}

// the loop function runs over and over again forever
void loop() {
  for(int i=0; i<16; i++){
    Serial.printf("%b ",i);
    digitalWrite(pins[0], !!(i&1));
    digitalWrite(pins[1], !!(i&2));
    digitalWrite(pins[2], !!(i&4));
    digitalWrite(pins[3], !!(i&8));
    delay(1000);
  }
  Serial.println("");
}
