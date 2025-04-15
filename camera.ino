#include "esp_camera.h"
#include "Arduino.h"
#include "FS.h"                // SD Card ESP32
#include "SD_MMC.h"            // SD Card ESP32
#include "soc/soc.h"           // Disable brownour problems
#include "soc/rtc_cntl_reg.h"  // Disable brownour problems
#include "driver/rtc_io.h"


#include "RTClib.h"
// define the number of bytes you want to access

// Pin definition for CAMERA_MODEL_AI_THINKER
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

int pictureNumber = 0;


const byte numChars = 128;
char receivedChars[numChars];

int newData = 0;

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector
 
  Serial.begin(19200);
  //Serial.setDebugOutput(true);
  //Serial.println();
  
  camera_config_t config;
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
  config.pixel_format = PIXFORMAT_RGB565; 
  
  if(psramFound()){
    config.frame_size = FRAMESIZE_VGA; // FRAMESIZE_ + QVGA|CIF|VGA|SVGA|XGA|SXGA|UXGA
    config.jpeg_quality = 10;
    config.fb_count = 1;
  } else {
    config.frame_size = FRAMESIZE_VGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }
  
  // Init Camera
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    //Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }
  
  //Serial.println("Starting SD Card");
  if(!SD_MMC.begin("/sdcard", true)){
    //Serial.println("SD Card Mount Failed");
    return;
  }
  
  uint8_t cardType = SD_MMC.cardType();
  if(cardType == CARD_NONE){
    //Serial.println("No SD Card attached");
    return;
  }

  pinMode(4, OUTPUT);
  digitalWrite(4, LOW);

  while (true) {
    recvWithStartEndMarkers();
    if (newData > 0) {
      if (newData == 1 && receivedChars[0]==0) {
        Serial.write((uint8_t) 0);
        break;
      }
      newData = 0;
    }
  }
}

static boolean recvInProgress = false;

static uint8_t startNum = 0;
static uint8_t endNum = 0;
static byte ndx = 0;
char startMarker = '<';
char endMarker = '>';
uint8_t rc;

void recvWithStartEndMarkers() {
  while (Serial.available() > 0 && newData == false) {
    rc = Serial.read();
    if (recvInProgress == true) {
        
      receivedChars[ndx] = rc;
      ndx++;
      if (ndx >= numChars) {
          ndx = numChars - 1;
      }
      if (rc == 254) {
        endNum += 1;

        if (endNum == 4) {
          endNum = 0;
          recvInProgress = false;
          
          newData = ndx-4;
          ndx = 0;
        }
      } else if (endNum > 0) {
        endNum = 0;
      }
    }
    else if (rc == 255) {
      startNum += 1;
      if (startNum == 4)  {
        recvInProgress = true;
        startNum = 0;
      }
    } else if (startNum > 0) {
      startNum = 0;
    }
  }
}
String path = "/Picture";
int repeat = 0;


void loop() {
  Serial.write((uint8_t) 0);
  Serial.setDebugOutput(false);

  camera_fb_t * fb = NULL;
  
  // Take Picture with Camera
  fb = esp_camera_fb_get();  
  if(!fb) {
    //Serial.println("Camera capture failed");
    return;
  }

  uint8_t * buf = NULL;
  size_t buf_len = 0;
  bool converted = frame2bmp(fb, &buf, &buf_len);
  esp_camera_fb_return(fb);
  if(!converted){
    //Serial.println("BMP conversion failed");
    return;
  }

  String new_path = "";
  while (true) {
    if (!Serial.available()) {
      break;
    }
    recvWithStartEndMarkers();
    if (newData > 0) {
      if (newData == 1 && receivedChars[0]==0) {
        Serial.write((uint8_t) 0);
      } else if (newData == sizeof(uint32_t)) {

        uint32_t t;
        memcpy(&t, receivedChars, sizeof(uint32_t));

        DateTime dt  = DateTime(t);

        char format[] = "YYMM_DD_hh_mm_ss";

        new_path = "/"+String(dt.toString(format));
      }
      newData = 0;
    }
  }
  
  if (new_path == "" || new_path == path) {
    new_path = path + "_" + String(repeat);

    repeat += 1;
  } else {
    path = new_path;

    repeat = 0;
  }
  new_path = new_path + ".bmp";
  newData = 0;
  recvInProgress = false;

  startNum = 0;
  endNum = 0;

  fs::FS &fs = SD_MMC; 
  //Serial.printf("Picture file name: %s\n", path.c_str());

  File file = fs.open(new_path.c_str(), FILE_WRITE);
  if(!file){
    //Serial.println("Failed to open file in writing mode");
  } 
  else {
    file.write(buf, buf_len); // payload (image), payload length
    //Serial.printf("Saved file to path: %s\n", path.c_str());
  }
  file.close();
  esp_camera_fb_return(fb); 
  free(buf);
  pictureNumber += 1;
}
