// All code in this file is from: 
// http://www.esp32learning.com/code/esp32-and-icm-20948-motiontracking-device-arduino-example.php
#include <Adafruit_ICM20X.h>
#include <Adafruit_ICM20948.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

#define dt_ms 50.0
#define dt_s dt_ms / 1000.0
#define rad_to_angle 180.0 / 3.141592 * 180.0/100.0  // for some reason, it will normally convert a full rotation to 200 without 180/100
#define angle_noise_threshold 0.05
#define linear_noise_threshold 0.0

// #define calibration_matrix_size 9
// float [calibration_matrix_size][3]calibration_matrix;

Adafruit_ICM20948 icm;

// orientation angles
float rx = 0.0;
float ry = 0.0;
float rz = 0.0;

// position coordinates
float px = 0.0;
float py = 0.0;
float pz = 0.0;

// position velocity
float vx = 0.0;
float vy = 0.0;
float vz = 0.0;

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

void calibrateG(){
  sensors_event_t accel;
  sensors_event_t gyro;
  sensors_event_t mag;
  sensors_event_t temp;
  icm.getEvent(&accel, &gyro, &temp, &mag);
  float ax = accel.acceleration.x;
  float ay = accel.acceleration.y;
  float az = accel.acceleration.z;
  double r = 9.81; // ground truth gravity
  double r2 = ax*ax + ay*ay + az*az;  // first calculation
  r = r + (r2 - r*r) / (r*2);  // Approximate the square root because
  // r = sqrt(r2);             // unfortunately this function just returns 0 at all times
  Serial.printf("Initial gravity magnitude r2 = %f\n", r2);
  Serial.printf("Initial gravity magnitude r = %f\n", r);
  Serial.printf("Post-estimate gravity magnitude r2 = %f\n", r*r);
  // while(1); // stall here for now
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

  // Calibrate to ignore gravity based on g experienced in first frame
  calibrateG(); 
}

void loop() {
  sensors_event_t accel;
  sensors_event_t gyro;
  sensors_event_t mag;
  sensors_event_t temp;
  icm.getEvent(&accel, &gyro, &temp, &mag);

  // Padding format from: https://stackoverflow.com/a/5325211
  /* Display the results (acceleration is measured in m/s^2) */
  float ax = accel.acceleration.x;
  float ay = accel.acceleration.y;
  float az = accel.acceleration.z;
  Serial.print("Acc ");
  Serial.printf("X: % 06.2f ", ax);
  Serial.printf("Y: % 06.2f ", ay);
  Serial.printf("Z: % 06.2f ", az);
  Serial.print(" m/s^2 ");

  // filter out noise
  if (abs(ax) < linear_noise_threshold) ax=0;
  if (abs(ax) < linear_noise_threshold) ay=0;

  // p_1 = R^1_0 p_0
  // R10 = [[c -s],[s c]]
  // x1 = c x0 - s y0
  // y1 = s x0 + c y0
  // theta = rz (normal to XY plane)
  // v_x = dt * a_x
  vx += dt_s * (cos(rz) * ax - sin(rz) * ay);
  vy += dt_s * (sin(rz) * ax + cos(rz) * ay);

  Serial.print("Vel ");
  Serial.printf("X: % 06.2f ", vx);
  Serial.printf("Y: % 06.2f ", vy);
  Serial.print(" m/s ");

  px += dt_s * vx;
  py += dt_s * vy;

  Serial.print("Pos ");
  Serial.printf("X: % 06.2f ", px * 100);
  Serial.printf("Y: % 06.2f ", py * 100);
  Serial.print(" cm  ");


  Serial.print("   Gyro ");
  // Serial.printf("X: % 05.2f ", gyro.gyro.x);
  // Serial.printf("Y: % 05.2f ", gyro.gyro.y);
  // Serial.printf("Z: % 05.2f ", gyro.gyro.z);
  // Serial.print(" rads/s ");
  if (abs(gyro.gyro.x) > angle_noise_threshold) rx += gyro.gyro.x * dt_s;
  if (abs(gyro.gyro.y) > angle_noise_threshold) ry += gyro.gyro.y * dt_s;
  if (abs(gyro.gyro.z) > angle_noise_threshold) rz += gyro.gyro.z * dt_s;

  Serial.print("  Angle ");
  Serial.printf("X: % 05.2f ", rx * rad_to_angle);
  Serial.printf("Y: % 05.2f ", ry * rad_to_angle);
  Serial.printf("Z: % 05.2f ", rz * rad_to_angle);
  Serial.print(" rads");

  // Serial.print("   Mag ");
  // Serial.printf("X: % 06.2f ", mag.magnetic.x);
  // Serial.printf("Y: % 06.2f ", mag.magnetic.y);
  // Serial.printf("Z: % 06.2f ", mag.magnetic.z);
  // Serial.print(" uT");

  Serial.print("   Temp ");
  Serial.print(temp.temperature);
  Serial.print(" C");

  Serial.println("");
  delay(dt_ms);
}
