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
#include "motor_control.hpp"
#include "motors.hpp"

// Model must be defined before including camera pins
#define CAMERA_MODEL_XIAO_ESP32S3 // Has PSRAM
#include "camera_pins.h"

#define USE_SD_CARD 1    // set to 1 (true) for saving images
#define perfTimeLog_en 0 // set to 1 (true) to enable more detailed logging of system state/timing
#define STOP_ON_SD_INIT_FAIL 1  // 1 -> if the SD card fails to initialize, stop the program

bool sd_loaded = false;

#define IMAGE_WIDTH  IMAGE_COLS
#define IMAGE_HEIGHT IMAGE_ROWS
#define TIME_FRAMES  2           // how many frames back in time are kept

// two instances of Two-dimensional array to hold the pixel values at consecutive time points
auto p_frame = new uint8_t [IMAGE_HEIGHT * IMAGE_WIDTH];  // prior
auto n_frame = new uint8_t [IMAGE_HEIGHT * IMAGE_WIDTH];  // next
auto corners = new uint8_t [IMAGE_HEIGHT * IMAGE_WIDTH];  // corners between the two frames

auto save_frame = new uint8_t [IMAGE_HEIGHT * IMAGE_WIDTH];  // a copy of the frame data for being written to an SD card without slowing down the pipeline
int saving = 0; // "Mutex" for sd card buffer save operations tracking

int times[TIME_FRAMES];  // tracks the age of the frames in the frames array. 

// measures of the frame-to-frame apparent motion of pixels in the view (+U -> Left, +V -> Up)
int16_t u_vals[IMAGE_HEIGHT * IMAGE_WIDTH];
int16_t v_vals[IMAGE_HEIGHT * IMAGE_WIDTH];

// Pins connected to each motor
int pins[] = {D0, D1, D2, D3};
int ctrl[] = {0,0};


void timeLog(String data) {
  if(Serial)
    Serial.printf("[%00d] - %s.\n", millis(), data);
}

// Wrapper to disable all performance time logs from one point
void perfTimeLog(String data){
  if(perfTimeLog_en){
    timeLog(data);
  }
}

int configureSD(){
    if(!USE_SD_CARD) return 0;

    // 0 -> good
    // non-zero -> error
    // Initialize SD card
    if(!SD.begin(21)){
      if(Serial) Serial.println("Card Mount Failed");
      return 1;
    }
    uint8_t cardType = SD.cardType();

    // Determine if the type of SD card is available
    if(cardType == CARD_NONE){
      if(Serial)
        Serial.println("No SD card attached");
        return 2;
    }

    sd_loaded = true;
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
void photo_save( void * params) {
  saving = 1;
  char filename[32];
  char fileU[32];
  char fileV[32];
  int time = times[1];
  sprintf(filename, "/raw/img/img%6d.bytes", time);
  sprintf(fileU,    "/raw/u/img%6d.U", time);
  sprintf(fileV,    "/raw/v/img%6d.V", time);
  perfTimeLog("File write starting");
  if(Serial)
      Serial.printf("Beginning of writing image %s\n", filename);

  File file = SD.open(filename, FILE_WRITE);    
  Serial.printf("File Object created.  Writing...\n");
  file.write(save_frame, IMAGE_WIDTH*IMAGE_HEIGHT);
  Serial.printf("File write complete\n");
  file.close();
  Serial.printf("File closed\n");
  // perfTimeLog("Image written");
  saving = 0;
  vTaskDelete(NULL);
}

TaskHandle_t sdImageTask;
void saveImage(){
  // Wrapper to send process to alternate core
  // https://randomnerdtutorials.com/esp32-dual-core-arduino-ide/
  xTaskCreatePinnedToCore(
    photo_save,    /* Task function. */
    "sdImageTask",  /* name of task. */
    10000,           /* Stack size of task */
    NULL,              /* parameter of the task */
    1,                  /* priority of the task */
    &sdImageTask,        /* Task handle to keep track of created task */
    SECONDARY_TASK_CORE); /* pin task to core 0 */
}


void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector

  Serial.begin(115200);
  while(!Serial && millis() < 1500); // When the serial monitor is turned on, the program starts to execute.  Or after 5 seconds for autonomous mode
  Serial.println("Started up");

  // motor config
  motorSetup();

  // peripherals if enabled
  if(initCamera())  while(true) {Serial.println("Failed to initialize camera"); delay(500);}
  if(configureSD()) while(STOP_ON_SD_INIT_FAIL && configureSD()) {Serial.println("Failed to initialize SD Card");delay(500);} 
}

void loop(){
  camera_fb_t * fb = esp_camera_fb_get();    // Take Picture with camera and put in buffer

  if (!fb) {
    Serial.println("Camera capture failed");
    return;
  }

  // perfTimeLog("Frame buffer verified");
  // Serial.printf("Camera buffer length: %d\n", fb->len);

  // Transfer pixel data from the image buffer to the 2D array
  for (int row = 0; row < IMAGE_HEIGHT; row++) {
    for (int col = 0; col < IMAGE_WIDTH; col++) {
      int index = (row * IMAGE_WIDTH) + col; // Calculate the index in the 1D buffer
      n_frame[index] = fb->buf[index];    // Copy the pixel value to the 2D array and put a 1 if above threshold, otherwise 0
    }
  }

  // Enable for testing, disable for high speed performance without SD card
  // if(USE_SD_CARD){photo_save();}
  if(USE_SD_CARD && sd_loaded && !saving){
    for (int row = 0; row < IMAGE_HEIGHT; row++) {
      for (int col = 0; col < IMAGE_WIDTH; col++) {
        int index = (row * IMAGE_WIDTH) + col; // Calculate the index in the 1D buffer
        save_frame[index] = fb->buf[index];
      }
    }
    saveImage();    
  }

  times[1] = millis();
  esp_camera_fb_return(fb);    // Release the image buffer

  findCorners(p_frame, corners);
  computeFlow(p_frame, n_frame, u_vals, v_vals, corners);  // compute the consequences
  

  // swap frames for next shot so that the one that was just taken is kept
  auto swap = n_frame;
  n_frame = p_frame;
  p_frame = swap;

  // update motor control outputs based on module policy
  motorControl(u_vals, v_vals, ctrl);

  // Execute the calculated signals
  setPower(ctrl[0], ctrl[1]);
  printf("Motors: L:%d,R:%d\n", ctrl[0],ctrl[1]);  
}
