#include <NeoSWSerial.h>
#include <SPI.h>
#include <SD.h>

NeoSWSerial ArduinoSerial(2, 3);

File dataFile;

const byte numChars = 128;
char receivedChars[numChars];

int newData = 0;

#define filename F("data.txt")

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
  Serial.begin(115200);
  Serial.println("<Arduino is ready>");
  ArduinoSerial.begin(19200);

  if (!SD.begin(10)) {
    Serial.println(F("SD card initialization failed!"));
    blink(1);
  }
  Serial.println(F("SD card initialization done."));
  if (SD.exists(filename)) {
    SD.remove(filename);
  }

  dataFile = SD.open(filename, FILE_WRITE);

  if (dataFile) {
    Serial.println(F("File successfully opened."));
  } else {
    Serial.println(F("Error opening file!"));
    blink(2);
  }
}

void loop() {
  recvWithStartEndMarkers();
  if (newData > 0) {
    // Serial.print(newData);
    // Serial.print("\t");
    // for (int i=0;i<newData;i++) {
    //   Serial.print((uint8_t) receivedChars[i]);
    //   Serial.print(" ");
    // }
    // Serial.println("");

    if (newData == 1 && receivedChars[0]==0) {
      ArduinoSerial.write((uint8_t) 0);
    } else if (newData == sizeof(struct dataStruct)) {
      struct dataStruct data;
      memcpy(&data, receivedChars, sizeof(struct dataStruct));

      dataFile.print(data.time); dataFile.print(",");
      dataFile.print(data.micro); dataFile.print(",");

      dataFile.print(data.bmp_temperature); dataFile.print(",");
      dataFile.print(data.bmp_pressure); dataFile.print(",");

      dataFile.print(data.mpu_yaw); dataFile.print(",");
      dataFile.print(data.mpu_pitch); dataFile.print(",");
      dataFile.print(data.mpu_roll); dataFile.print(",");

      dataFile.print(data.mpu_accx); dataFile.print(",");
      dataFile.print(data.mpu_accy); dataFile.print(",");
      dataFile.print(data.mpu_accz); dataFile.print(",");

      dataFile.print(data.mpu_laccx); dataFile.print(",");
      dataFile.print(data.mpu_laccy); dataFile.print(",");
      dataFile.print(data.mpu_laccz); dataFile.print(",");

      dataFile.print(data.mpu_gyrox); dataFile.print(",");
      dataFile.print(data.mpu_gyroy); dataFile.print(",");
      dataFile.print(data.mpu_gyroz); dataFile.print(",");

      dataFile.print(data.mpu_magx); dataFile.print(",");
      dataFile.print(data.mpu_magy); dataFile.print(",");
      dataFile.print(data.mpu_magz); dataFile.print(",");

      dataFile.println(data.mpu_temperature);
      dataFile.flush();

      Serial.print(data.time); Serial.print(",");
      Serial.print(data.micro); Serial.print(",");

      Serial.print(data.bmp_temperature); Serial.print(",");
      Serial.print(data.bmp_pressure); Serial.print(",");

      Serial.print(data.mpu_yaw); Serial.print(",");
      Serial.print(data.mpu_pitch); Serial.print(",");
      Serial.print(data.mpu_roll); Serial.print(",");

      Serial.print(data.mpu_accx); Serial.print(",");
      Serial.print(data.mpu_accy); Serial.print(",");
      Serial.print(data.mpu_accz); Serial.print(",");

      Serial.print(data.mpu_laccx); Serial.print(",");
      Serial.print(data.mpu_laccy); Serial.print(",");
      Serial.print(data.mpu_laccz); Serial.print(",");

      Serial.print(data.mpu_gyrox); Serial.print(",");
      Serial.print(data.mpu_gyroy); Serial.print(",");
      Serial.print(data.mpu_gyroz); Serial.print(",");

      Serial.print(data.mpu_magx); Serial.print(",");
      Serial.print(data.mpu_magy); Serial.print(",");
      Serial.print(data.mpu_magz); Serial.print(",");

      Serial.println(data.mpu_temperature);
    }

    newData = 0;
  }
}

void recvWithStartEndMarkers() {
  static boolean recvInProgress = false;

  static uint8_t startNum = 0;
  static uint8_t endNum = 0;
  static byte ndx = 0;
  char startMarker = '<';
  char endMarker = '>';
  uint8_t rc;

  while (ArduinoSerial.available() > 0 && newData == false) {
    rc = ArduinoSerial.read();
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


void blink(int count) {
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
