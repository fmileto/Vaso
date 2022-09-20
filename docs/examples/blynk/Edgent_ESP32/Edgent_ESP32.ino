
// Fill-in information from your Blynk Template here
#define BLYNK_TEMPLATE_ID "TMPLYULgFXUJ"
#define BLYNK_DEVICE_NAME "Vaso"
#include <TCSoil.h>

#define SOIL_PIN_1 32
#define SOIL_PIN_2 33
#define SOIL_PIN_3 34

#define BLYNK_FIRMWARE_VERSION        "0.1.0"

#define BLYNK_PRINT Serial
//#define BLYNK_DEBUG

#define APP_DEBUG

// Uncomment your board, or configure a custom board in Settings.h
//#define USE_WROVER_BOARD
//#define USE_TTGO_T7
//#define USE_TTGO_T_OI
//#define USE_ESP32C3_DEV_MODULE
//#define USE_ESP32S2_DEV_KIT

#include "BlynkEdgent.h"

BlynkTimer timer;

Soil soil_1(SOIL_PIN_1, "Sens1");
Soil soil_2(SOIL_PIN_2, "Sens2");
Soil soil_3(SOIL_PIN_3, "Sens3");
int soilAvg = 0;
int waterCalibration = 897;

BLYNK_WRITE(V2) {
  waterCalibration = param.asInt();
  Serial.print("New water calibration: "); Serial.println(waterCalibration);
  soil_1.calibrateWater(waterCalibration);
  soil_2.calibrateWater(waterCalibration);
  soil_3.calibrateWater(waterCalibration);
}

BLYNK_WRITE(V0) {
  waterCalibration = param.asInt();
  Serial.print("New water calibration: "); Serial.println(waterCalibration);
  soil_1.calibrateWater(waterCalibration);
  soil_2.calibrateWater(waterCalibration);
  soil_3.calibrateWater(waterCalibration);
}

BLYNK_WRITE(V3) {
  int led = param.asInt();
  Serial.print("Led change from blynk "); Serial.println(led);
  digitalWrite(LED_BUILTIN, led == 1 ? 0 : 1);
}

void soilMoistureMeasure()
{
  int soilPercent_1 = soil_1.readHumidityPercent();
  int soilPercent_2 = soil_2.readHumidityPercent();
  int soilPercent_3 = soil_3.readHumidityPercent();
  soilAvg = (soilPercent_1 + soilPercent_2 + soilPercent_3) / 3;
  Serial.print("Soil 1: ");
  Serial.println(soilPercent_1);
  Serial.print("Soil 2: ");
  Serial.println(soilPercent_2);
  Serial.print("Soil 3: ");
  Serial.println(soilPercent_3);
  Serial.print("Soil AVG: ");
  Serial.println(soilAvg);
}

void myTimerEvent()
{
  // You can send any value at any time.
  // Please don't send more that 10 values per second.
  soilMoistureMeasure();

  Blynk.virtualWrite(V0, soilAvg);
}

void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, 1);
  Serial.begin(115200);
  delay(100);

  BlynkEdgent.begin();

  // Setup a function to be called every second
  timer.setInterval(5000L, myTimerEvent);
}

void loop() {
  BlynkEdgent.run();
  timer.run(); // Initiates BlynkTimer
}
