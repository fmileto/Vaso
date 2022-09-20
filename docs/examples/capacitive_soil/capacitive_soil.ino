#include <TCSoil.h>
const byte moistureSensorPowerPin = 19;
#define SOIL_PIN_1 33

Soil soil_1(SOIL_PIN_1, "Sens1");

void setup() {
  delay(500); //delay here is required to wake up reliably
  // put your setup code here, to run once:
   pinMode(moistureSensorPowerPin, OUTPUT);
 Serial.begin(115200);
  Serial.println("Begin soil");
  soil_1.begin();
  digitalWrite(moistureSensorPowerPin, HIGH); // power up the sensor

}

void soilMoistureMeasure()
{
  int soilPercent_1 = soil_1.readHumidityPercent();
  Serial.print("Soil 1: ");
  Serial.println(soilPercent_1);
}

void loop() {
  // put your main code here, to run repeatedly:
  soilMoistureMeasure();
  Serial.println("==========");

  delay(1000);
}
