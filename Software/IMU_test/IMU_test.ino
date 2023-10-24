// All code in this file is from: 
// http://www.esp32learning.com/code/esp32-and-icm-20948-motiontracking-device-arduino-example.php
#include <Adafruit_ICM20X.h>
#include <Adafruit_ICM20948.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

Adafruit_ICM20948 icm;

void printAccelerometerConfig(){
  // Accelerometer
  Serial.print("Accelerometer range set to: ");
  switch (icm.getAccelRange()) {
  case ICM20948_ACCEL_RANGE_2_G:
    Serial.println("+-2G");
    break;
  case ICM20948_ACCEL_RANGE_4_G:
    Serial.println("+-4G");
    break;
  case ICM20948_ACCEL_RANGE_8_G:
    Serial.println("+-8G");
    break;
  case ICM20948_ACCEL_RANGE_16_G:
    Serial.println("+-16G");
    break;
  }


  uint16_t accel_divisor = icm.getAccelRateDivisor();
  float accel_rate = 1125 / (1.0 + accel_divisor);

  Serial.print("Accelerometer data rate divisor set to: ");
  Serial.println(accel_divisor);
  Serial.print("Accelerometer data rate (Hz) is approximately: ");
  Serial.println(accel_rate);
}

void printGyroConfig(){
  Serial.print("Gyro range set to: ");
  switch (icm.getGyroRange()) {
  case ICM20948_GYRO_RANGE_250_DPS:
    Serial.println("250 degrees/s");
    break;
  case ICM20948_GYRO_RANGE_500_DPS:
    Serial.println("500 degrees/s");
    break;
  case ICM20948_GYRO_RANGE_1000_DPS:
    Serial.println("1000 degrees/s");
    break;
  case ICM20948_GYRO_RANGE_2000_DPS:
    Serial.println("2000 degrees/s");
    break;
  }


  uint8_t gyro_divisor = icm.getGyroRateDivisor();
  float gyro_rate = 1100 / (1.0 + gyro_divisor);

  Serial.print("Gyro data rate divisor set to: ");
  Serial.println(gyro_divisor);
  Serial.print("Gyro data rate (Hz) is approximately: ");
  Serial.println(gyro_rate);
}

void setup() {
  Serial.begin(115200);
  while (!Serial) ;
  Serial.println("Starting IMU test code");
  if(!icm.begin_I2C()){
    Serial.println("Failed to establish I2C communication with IMU");
    while(true) ;
  }

  Serial.println("ICM20948 Found!");
  printAccelerometerConfig();
  printGyroConfig();
}

void loop() {
  sensors_event_t accel;
  sensors_event_t gyro;
  sensors_event_t mag;
  sensors_event_t temp;
  icm.getEvent(&accel, &gyro, &temp, &mag);

  // Padding format from: https://stackoverflow.com/a/5325211
  /* Display the results (acceleration is measured in m/s^2) */
  Serial.print("Acc ");
  Serial.printf("X: % 05.2f ", accel.acceleration.x);
  Serial.printf("Y: % 05.2f ", accel.acceleration.y);
  Serial.printf("Z: % 05.2f ", accel.acceleration.z);
  Serial.print(" m/s^2 ");

  Serial.print("   Gyro ");
  Serial.printf("X: % 05.2f ", gyro.gyro.x);
  Serial.printf("Y: % 05.2f ", gyro.gyro.y);
  Serial.printf("Z: % 05.2f ", gyro.gyro.z);
  Serial.print(" rads/s ");

  Serial.print("   Mag ");
  Serial.printf("X: % 05.2f ", mag.magnetic.x);
  Serial.printf("Y: % 05.2f ", mag.magnetic.y);
  Serial.printf("Z: % 05.2f ", mag.magnetic.z);
  Serial.print(" uT");

  Serial.print("   Temp ");
  Serial.print(temp.temperature);
  Serial.println(" C");

  delay(500);
}
