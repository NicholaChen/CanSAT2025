#include <NeoSWSerial.h>
#include "RTClib.h"


#include <Adafruit_BMP280.h>

#include "MPU9250.h"

Adafruit_BMP280 bmp;

MPU9250 mpu;

#define filename F("data.txt")

NeoSWSerial SDSerial(2, 3);
NeoSWSerial CameraSerial(5, 4);


RTC_DS3231 rtc;

struct dataStruct {
  uint32_t time;
  unsigned long micro;
  
  float bmp_temperature;
  float bmp_pressure;
  
  float mpu_yaw;
  float mpu_pitch;
  float mpu_roll;

  float mpu_accx;
  float mpu_accy;
  float mpu_accz;

  float mpu_laccx;
  float mpu_laccy;
  float mpu_laccz;

  float mpu_gyrox;
  float mpu_gyroy;
  float mpu_gyroz;

  float mpu_magx;
  float mpu_magy;
  float mpu_magz;

  float mpu_temperature;
};

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Wire.begin();
  
  SDSerial.begin(19200);
  CameraSerial.begin(19200);

  if (!bmp.begin()) {
    blink(3);
  }
  
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL, Adafruit_BMP280::SAMPLING_X2, Adafruit_BMP280::SAMPLING_X16, Adafruit_BMP280::FILTER_X16, Adafruit_BMP280::STANDBY_MS_500);

  
  if (!mpu.setup(0x68)) {
    blink(4);
  }
  mpu.setAccBias(
      871.70, -184.70, -877.00);
  mpu.setGyroBias(
      267.77, 69.57, 66.77);
  mpu.setMagBias(
      102.83, 164.69, -763.65);
  mpu.setMagScale(
      1.02, 1.64, 0.71);


  if (!rtc.begin()) {
    blink(5);
  }
  
  SDSerial.listen();

  while (1) {
    for (int i = 0; i < 4; i++) SDSerial.write(255);
    SDSerial.write((uint8_t) 0);
    for (int i = 0; i < 4; i++) SDSerial.write(254);
    if (SDSerial.available() > 0) {
      if (SDSerial.read() == 0) {
        while (SDSerial.available() > 0) {
          SDSerial.read();
        }
        break;
      }
    }
    delay(50);
  }

  CameraSerial.listen();
  while (1) {
    for (int i = 0; i < 4; i++) CameraSerial.write(255);
    CameraSerial.write((uint8_t) 0);
    for (int i = 0; i < 4; i++) CameraSerial.write(254);
    if (CameraSerial.available() > 0) {
      if (CameraSerial.read() == 0) {
        while (CameraSerial.available() > 0) {
          CameraSerial.read();
        }
        break;
      }
    }
    delay(50);
  }
}
dataStruct data;
DateTime t;

void loop() {
  unsigned long m = millis();
  static uint32_t prev_ms = m;

  mpu.update();

  if (m > prev_ms + 25) {
    SDSerial.listen();
    t = rtc.now();
    data.time = t.unixtime();
    data.micro = micros();

    data.bmp_temperature = bmp.readTemperature();
    data.bmp_pressure = bmp.readPressure();

    data.mpu_yaw = mpu.getYaw();
    data.mpu_pitch = mpu.getPitch();
    data.mpu_roll = mpu.getRoll();

    data.mpu_accx = mpu.getAccX();
    data.mpu_accy = mpu.getAccY();
    data.mpu_accz = mpu.getAccZ();

    data.mpu_laccx = mpu.getLinearAccX();
    data.mpu_laccy = mpu.getLinearAccY();
    data.mpu_laccz = mpu.getLinearAccZ();

    data.mpu_gyrox = mpu.getGyroX();
    data.mpu_gyroy = mpu.getGyroY();
    data.mpu_gyroz = mpu.getGyroZ();

    data.mpu_magx = mpu.getMagX();
    data.mpu_magy = mpu.getMagY();
    data.mpu_magz = mpu.getMagZ();

    data.mpu_temperature = mpu.getTemperature();

    for (int i = 0; i < 4; i++) SDSerial.write(255);
    SDSerial.write((const char*) &data, sizeof(struct dataStruct));
    for (int i = 0; i < 4; i++) SDSerial.write(254);
    prev_ms = m;
    
    CameraSerial.listen();
  }
  
  if (CameraSerial.available() > 0) {
    while (CameraSerial.available() > 0) {
      CameraSerial.read();
    }
    for (int i = 0; i < 4; i++) CameraSerial.write(255);
    CameraSerial.write((const char*) t.unixtime(), sizeof(uint32_t));
    for (int i = 0; i < 4; i++) CameraSerial.write(254);
  }
}

void blink(uint8_t count) {
  while (1) {
    for (uint8_t i=0;i<count;i++) {
      digitalWrite(LED_BUILTIN, HIGH);
      delay(75);
      digitalWrite(LED_BUILTIN, LOW);
      delay(75);
    }
    delay(200);
  }
}
