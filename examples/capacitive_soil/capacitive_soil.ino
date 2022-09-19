#include <TCSoil.h>
#define SOIL_PIN_1 32
#define SOIL_PIN_2 33
#define SOIL_PIN_3 34

Soil soil_1(SOIL_PIN_1, "Sens1");
Soil soil_2(SOIL_PIN_2, "Sens2");
Soil soil_3(SOIL_PIN_3, "Sens3");

void setup() {
  delay(500); //delay here is required to wake up reliably
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("Begin soil");
  soil_1.begin();
  soil_2.begin();
  soil_3.begin();

}

void soilMoistureMeasure()
{
  int soilPercent_1 = soil_1.readHumidityPercent();
  int soilPercent_2 = soil_2.readHumidityPercent();
  int soilPercent_3 = soil_3.readHumidityPercent();
  int soilAvg = (soilPercent_1 + soilPercent_2 + soilPercent_3) / 3;
  Serial.print("Soil 1: ");
  Serial.println(soilPercent_1);
  Serial.print("Soil 2: ");
  Serial.println(soilPercent_2);
  Serial.print("Soil 3: ");
  Serial.println(soilPercent_3);
  Serial.print("Soil AVG: ");
  Serial.println(soilAvg);
}

void loop() {
  // put your main code here, to run repeatedly:
  soilMoistureMeasure();
  Serial.println("==========");

  delay(1000);
}
