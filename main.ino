#include <SPI.h>
#include <SD.h>


#include <Adafruit_BMP280.h>


Adafruit_BMP280 bmp;

File dataFile;

const char filename[] = "data.txt";
void setup() {
  pinMode(LED_BUILTIN, OUTPUT);

  Serial.begin(115200);
  while (!Serial) {
    ;
  }


  Serial.println("Initializing SD card");

  if (!SD.begin(10)) {
    Serial.println("SD card initialization failed!");
    blink(1);
  }
  Serial.println("SD card initialization done.");
  if (SD.exists(filename)) {
    SD.remove(filename);
  }

  dataFile = SD.open(filename, FILE_WRITE);

  if (dataFile) {
    Serial.println("File successfully opened.");
  } else {
    Serial.println("Error opening file!");
    blink(2);
  }

  dataFile.println("Time (ms),BMP280 Temperature (°C),BMP280 Pressure (Pa)");

  Serial.println("Initializing BMP280");

  if (!bmp.begin()) {
    Serial.println("BMP280 initialization failed!");
    blink(3);
  }
  Serial.println("BMP280 initialization done.");

  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,
                  Adafruit_BMP280::SAMPLING_X2,
                  Adafruit_BMP280::SAMPLING_X16,
                  Adafruit_BMP280::FILTER_X16,
                  Adafruit_BMP280::STANDBY_MS_500);
}

void loop() {
  unsigned long m = millis();

  float bmpTemperature;
  float bmpPressure;

  
  
  bmpTemperature = bmp.readTemperature();
  bmpPressure = bmp.readPressure();
  dataFile.print(m);
  dataFile.print(",");
  dataFile.print(bmpTemperature);
  dataFile.print(",");
  dataFile.println(bmpPressure);

  dataFile.flush();

  Serial.print(bmpTemperature);
  Serial.print(",");
  Serial.println(bmpPressure);

}

void blink(int count) {
  SPI.end();

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