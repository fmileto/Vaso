/*const int MOIST_PIN = 32;

void setup(){
  Serial.begin(9600);
  Serial.println("START ===============");
  pinMode(MOIST_PIN, INPUT);
  delay(1500);
}

void loop(){
  int m = analogRead(MOIST_PIN);
  Serial.println(m);
  delay(300);
}
*/
#include "DHT.h"

#define DHT_PIN 32
#define DHTTYPE DHT22

DHT dht(DHT_PIN, DHTTYPE);
float t;

void setup()  {
  Serial.begin(9600);\
  Serial.println("START ===============");
  delay(1500);
  dht.begin();
}

void readEnvTemperature() {
  t = dht.readTemperature();
  Serial.print("Temp: ");
  Serial.print(t);
  Serial.println("C");
}

void loop(){
  readEnvTemperature();
  Serial.println("=============");
  delay(2000);
}
