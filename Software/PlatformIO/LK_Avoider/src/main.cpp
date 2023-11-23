#include <Arduino.h>

#include "esp_camera.h"
#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include "soc/soc.h"           // Disable brownout problems
#include "soc/rtc_cntl_reg.h"  // Disable brownout problems
#include "driver/rtc_io.h"
#include "img_converters.h" // see https://github.com/espressif/esp32-camera/blob/master/conversions/include/img_converters.h

#include "optical_flow.hpp"

// Model must be defined before including camera pins
#define CAMERA_MODEL_XIAO_ESP32S3 // Has PSRAM
#include "camera_pins.h"


// https://files.seeedstudio.com/wiki/SeeedStudio-XIAO-ESP32S3/img/2.jpg
// Digital pins 6 and 7
// For testing only
#define L_MOTOR_PIN 43
#define R_MOTOR_PIN 44

#define USE_SD_CARD 0    // set to 1 (true) for saving images
#define perfTimeLog_en 0 // set to 1 (true) to enable more detailed logging of system state/timing

#define IMAGE_WIDTH  IMAGE_COLS
#define IMAGE_HEIGHT IMAGE_ROWS
#define TIME_FRAMES  2           // how many frames back in time are kept

#define OPTICAL_FLOW_INTENSITY_THRESHOLD 5     // the magnitude of flow after which point, it counts toward taking evasive behavior
#define OPTICAL_FLOW_QUANTITY_THRESHOLD 15     // the number of flow points above the threshold, requiring at least this many to take evasive action


// two instances of Two-dimensional array to hold the pixel values at consecutive time points
auto p_frame = new uint8_t [IMAGE_HEIGHT * IMAGE_WIDTH];  // prior
auto n_frame = new uint8_t [IMAGE_HEIGHT * IMAGE_WIDTH];  // next

int times[TIME_FRAMES];  // tracks the age of the frames in the frames array. 

// measures of the frame-to-frame apparent motion of pixels in the view (+U -> Left, +V -> Up)
int16_t u_vals[IMAGE_HEIGHT * IMAGE_WIDTH];
int16_t v_vals[IMAGE_HEIGHT * IMAGE_WIDTH];


void timeLog(String data) {
  if(Serial)
    Serial.printf("%d - %s.\n", millis(), data);
}

// Wrapper to disable all performance time logs from one point
void perfTimeLog(String data){
  if(perfTimeLog_en){
    timeLog(data);
  }
}

int configureSD(){
    // 0 -> good
    // non-zero -> error
    // Initialize SD card
    if(!SD.begin(21)){
      if(Serial)
        Serial.println("Card Mount Failed");
        return 1;
    }
    uint8_t cardType = SD.cardType();

    // Determine if the type of SD card is available
    if(cardType == CARD_NONE){
      if(Serial)
        Serial.println("No SD card attached");
        return 2;
    }

    if(Serial){
      Serial.print("SD Card Type: ");
      if(cardType == CARD_MMC){
          Serial.println("MMC");
      } else if(cardType == CARD_SD){
          Serial.println("SDSC");
      } else if(cardType == CARD_SDHC){
          Serial.println("SDHC");
      } else {
          Serial.println("UNKNOWN");
      }
    }

    return 0;
}

int initCamera() {
  // 0 -> success
  // non-zero -> error
  camera_config_t config; //setting up configuration 
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.frame_size = FRAMESIZE_96X96; // _QVGA -> (320 x 240)   _96X96 -> (96 x 96)
  config.pixel_format = PIXFORMAT_GRAYSCALE; // changed to grayscale
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.jpeg_quality = 12; //this can be adjusted to create lower or higher quality images
  config.fb_count = 1;

  // camera initialize, will need to remove some of these things for the robot itself
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    if(Serial)
      Serial.printf("Camera init failed with error 0x%x", err);
    return err;
  }
  return 0;
}

// based on take_photos writeFile
void photo_save() {
    char filename[32];
    char fileU[32];
    char fileV[32];
    int time = times[1];
    sprintf(filename, "/image%d.bytes", time);
    sprintf(fileU, "/image%d.U", time);
    sprintf(fileV, "/image%d.V", time);
    perfTimeLog("File write starting");
    if(Serial)
        Serial.printf("Beginning of writing image %s\n", filename);

    File file = SD.open(filename, FILE_WRITE);    
    file.write(n_frame, IMAGE_WIDTH*IMAGE_HEIGHT);
    file.close();

    perfTimeLog("Image written");
}

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector

  Serial.begin(115200);
  while(!Serial && millis() < 5000); // When the serial monitor is turned on, the program starts to execute.  Or after 5 seconds for autonomous mode
  Serial.println("Started up");

  // motor config
  pinMode(R_MOTOR_PIN, OUTPUT);
  pinMode(L_MOTOR_PIN, OUTPUT);
  
  if(initCamera()) Serial.println("Failed to initialize camera");
  if(configureSD()) Serial.println("Failed to initialize SD Card");
}

void loop(){
  // setting up a pointer to the frame buffer
  camera_fb_t * fb = NULL;
  
  // if(Serial)
  //     Serial.printf("\n\n\t\tCYCLE START\t %d\n", millis());
  // Take Picture with camera and put in buffer
  fb = esp_camera_fb_get();

  if (!fb) {
    Serial.println("Camera capture failed");
    return;
  }


  perfTimeLog("Frame buffer verified");
  // Serial.printf("Camera buffer length: %d\n", fb->len);

  // Transfer pixel data from the image buffer to the 2D array
  for (int row = 0; row < IMAGE_HEIGHT; row++) {
    for (int col = 0; col < IMAGE_WIDTH; col++) {
      int index = (row * IMAGE_WIDTH) + col; // Calculate the index in the 1D buffer
      n_frame[index] = fb->buf[index];    // Copy the pixel value to the 2D array and put a 1 if above threshold, otherwise 0
    }
  }
  times[1] = millis();
  // Release the image buffer
  esp_camera_fb_return(fb);

  // compute the consequences
  computeFlow(p_frame, n_frame, u_vals, v_vals);
  
  // Enable for testing, disable for high speed performance without SD card
  if(USE_SD_CARD){
      photo_save();
  }

  // swap frames for next shot so that the one that was just taken is kept
  auto swap = n_frame;
  n_frame = p_frame;
  p_frame = swap;

  // count flow in each half of the screen
  int leftSum = 0;
  int rightSum = 0;
  for(int r=0; r < IMAGE_ROWS; r++){
    for(int c=0; c < IMAGE_COLS; c++){
      int idx = r * IMAGE_COLS + c;
      int mag = u_vals[idx]*u_vals[idx] + v_vals[idx]*v_vals[idx];                      // magnitude squared, for computational simplicity
      if(mag >= OPTICAL_FLOW_INTENSITY_THRESHOLD * OPTICAL_FLOW_INTENSITY_THRESHOLD){   // compare against the threshold
          if(c < IMAGE_COLS/2) {leftSum++;} else {rightSum++;}                          // contribute to the side's sum
      }
    }
  }

  // print how many flow points are above the turning threshold
  Serial.printf("L: %d\tR:%d\t", leftSum, rightSum);
  int lMotor,rMotor;
  lMotor=rMotor=1; // start with both enabled
  if(leftSum > OPTICAL_FLOW_QUANTITY_THRESHOLD) rMotor = 0;
  if(rightSum > OPTICAL_FLOW_QUANTITY_THRESHOLD) lMotor = 0;
  Serial.printf("Motors: L:%d,R:%d\n", lMotor,rMotor);
  digitalWrite(L_MOTOR_PIN, lMotor);
  digitalWrite(R_MOTOR_PIN, rMotor);
  // send control signal outputs
  // delay(200);
}
