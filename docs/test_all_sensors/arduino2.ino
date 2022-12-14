#include "DHT.h"

#define MOIST_APIN A0
#define DHT_APIN 4
#define LDR_APIN A2
#define RELAY 12
#define ENABLE_MOIST 2
#define MIN_MOIST 240
#define MAX_MOIST 1024

#define MAX_ADC_READING           1023
#define ADC_REF_VOLTAGE           5.0
#define REF_RESISTANCE            5030  // measure this for best results
#define LUX_CALC_SCALAR           12518931
#define LUX_CALC_EXPONENT         -1.405
#define DHTTYPE DHT22

float R1 = 10000;
float c1 = 1.009249522e-03, c2 = 2.378405444e-04, c3 = 2.019202697e-07;

int TARGET_MOISTURE = 30;
DHT dht(DHT_APIN, DHTTYPE);

void setup()  {
  Serial.begin(9600);
  Serial.println("START ===============");
  delay(1500);
  pinMode(RELAY, OUTPUT);
  pinMode(LDR_APIN, INPUT);
  dht.begin();
}

void irrigate(int currentMoisture, int targetMoisture) {
    Serial.println("Start irrigate");
    digitalWrite(RELAY, HIGH);
    delay(3000);
    int moisture = readMoistureHumidity();
    if(moisture < targetMoisture){
      return irrigate(moisture, targetMoisture);
    } else {
      digitalWrite(RELAY, LOW);
      Serial.println("Stop irrigate");
      return;
    }
}

void readEnvTemperature() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  Serial.print("ENV - Humidity: ");
  Serial.print(h);
  Serial.print("%, Temp: ");
  Serial.print(t);
  //Serial.print("Humidity: " + String(DHT.humidity) + ", Temperature: " + String(DHT.temperature));
  Serial.println("C");
}

int readLight() {
    
  int   ldrRawData;
  float resistorVoltage, ldrVoltage;
  float ldrResistance;
  float ldrLux;
  ldrRawData = analogRead(LDR_APIN);
  // RESISTOR VOLTAGE_CONVERSION
  // Convert the raw digital data back to the voltage that was measured on the analog pin
  resistorVoltage = (float)ldrRawData / MAX_ADC_READING * ADC_REF_VOLTAGE;

  // voltage across the LDR is the 5V supply minus the 5k resistor voltage
  ldrVoltage = ADC_REF_VOLTAGE - resistorVoltage;
  
  // LDR_RESISTANCE_CONVERSION
  // resistance that the LDR would have for that voltage  
  ldrResistance = ldrVoltage/resistorVoltage * REF_RESISTANCE;
  
  // LDR_LUX
  // Change the code below to the proper conversion from ldrResistance to
  // ldrLux
  ldrLux = LUX_CALC_SCALAR * pow(ldrResistance, LUX_CALC_EXPONENT);

  Serial.println("Light: " + String(ldrLux) + " lux");
}

int readMoistureHumidity(){
  // turn on moist sens
  // reset relay
  digitalWrite(ENABLE_MOIST, HIGH);
  delay(200);
  int h2oValRaw = 0;
  for(int i = 0; i<10; i++){
    h2oValRaw += analogRead(MOIST_APIN);
    delay(50);
  }
  h2oValRaw /= 10; 
  // turn off moist
  digitalWrite(ENABLE_MOIST, LOW);

  int moisture = map(h2oValRaw, MIN_MOIST, MAX_MOIST, 100, 0);
  moisture = constrain(moisture, 0, 100);
  Serial.println("Umidit??: " + String(moisture) + "%\traw: " + String(h2oValRaw)); //Stampa a schermo il valore
  return moisture;
}

void loop(){
  //int moisture = readMoistureHumidity();
  readEnvTemperature();
  //readLight();
  Serial.println("=============");
  //if (moisture < TARGET_MOISTURE) {
  //  irrigate(moisture, TARGET_MOISTURE);
 // }
  delay(2000);
}
