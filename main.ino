#define DEBUG

//#include <SPI.h>
//#include <SD.h>


#include <Adafruit_BMP280.h>

#include "MPU9250.h"
#include "eeprom_utils.h"

//File dataFile;

Adafruit_BMP280 bmp;

MPU9250 mpu;


#define filename F("data.txt")

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Wire.begin();
  
  Serial.begin(115200);
  
  while (!Serial) {
    ;
  }


  #if defined(DEBUG) 
    Serial.println(F("Initializing SD card")); 
  #endif

  // if (!SD.begin(10)) {
  //   #if defined(DEBUG)
  //     Serial.println(F("SD card initialization failed!"));
  //   #endif
  //   blink(1);
  // }
  // #if defined(DEBUG)
  //   Serial.println(F("SD card initialization done."));
  // #endif
  // if (SD.exists(filename)) {
  //   SD.remove(filename);
  // }

  // dataFile = SD.open(filename, FILE_WRITE);

  // if (dataFile) {
  //   #if defined(DEBUG)
  //     Serial.println(F("File successfully opened."));
  //   #endif
  // } else {
  //   #if defined(DEBUG)
  //     Serial.println(F("Error opening file!"));
  //   #endif
  //   blink(2);
  // }

  // dataFile.println(F("Time (ms),BMP280 Temperature (°C),BMP280 Pressure (Pa)"));

  #if defined(DEBUG)
    Serial.println(F("Initializing BMP280"));
  #endif

  if (!bmp.begin()) {
    #if defined(DEBUG)
      Serial.println(F("BMP280 initialization failed!"));
    #endif
    blink(3);
  }
  #if defined(DEBUG)
    Serial.println(F("BMP280 initialization done."));
  #endif
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL, Adafruit_BMP280::SAMPLING_X2, Adafruit_BMP280::SAMPLING_X16, Adafruit_BMP280::FILTER_X16, Adafruit_BMP280::STANDBY_MS_500);

  #if defined(DEBUG)
    Serial.println(F("Initializing MPU9250"));
  #endif
  if (!mpu.setup(0x68)) {
    #if defined(DEBUG)
      Serial.println(F("MPU9250 initialization failed!"));
    #endif
    blink(4);
  }
  #if defined(DEBUG)
    Serial.println(F("MPU9250 initialization done."));
  #endif
  loadCalibration(); // load saved calibration for MPU9250
}

void loop() {
  unsigned long m = millis();
  mpu.update();

  // dataFile.print(m);
  // dataFile.print(F(","));
  // dataFile.print(bmp.readTemperature());
  // dataFile.print(F(","));
  // dataFile.print(bmp.readPressure());
  // dataFile.print(F(","));
  // dataFile.print(mpu.getYaw());
  // dataFile.print(F(","));
  // dataFile.print(mpu.getPitch());
  // dataFile.print(F(","));
  // dataFile.println(mpu.getRoll());
  Serial.print(m);
  Serial.print(F(","));
  Serial.print(bmp.readTemperature());
  Serial.print(F(","));
  Serial.print(bmp.readPressure());
  Serial.print(F(","));
  Serial.print(mpu.getYaw());
  Serial.print(F(","));
  Serial.print(mpu.getPitch());
  Serial.print(F(","));
  Serial.println(mpu.getRoll());
  //dataFile.flush();

}

void blink(int count) {
  //SPI.end();

  while (1) {
    for (int i=0;i<count;i++) {
      digitalWrite(LED_BUILTIN, HIGH);
      delay(75);
      digitalWrite(LED_BUILTIN, LOW);
      delay(75);
    }
    delay(200);
  }
}
