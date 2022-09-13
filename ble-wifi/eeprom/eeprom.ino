#include "EEPROM.h"
#define EEPROM_SIZE 128

#include <WiFi.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#include <HTTPClient.h>
#include <Arduino_JSON.h>

const String serverHostName = "http://192.168.1.40:7450";
//const String serverHostName = "http://iot.theconnective.team";

BLEServer *pServer = NULL;
BLECharacteristic *pCharacteristic = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;

const int wifiAddr = 0;

bool hasWiFiCredentials = false;
bool isWiFiConnected = false;

int modeIdx;

// PIN CONFIGURATION
const byte buttonPin = 0;        // If changed, the pin also has to be changed in the Sleep function
const byte batteryLevelPin = 32; // voltage divider with 1MOhm and 30KOhm Resistor used to reduce maximum volatge at this pin (3.4V to 3.33V) --> at minimum operating voltage of 2.55V(???) readings at this pin would be 2,47V
const byte pumpPowerPin = 23;
const byte moistureSensorPowerPin = 19;
const byte moistureSensorSignalPin = 33;
const byte waterLevelGroundPin = 35;               // lowest cable in tank, connect 100kohm Resistor to Ground parallel as pulldown resitor
const byte waterLevelPin[] = {13, 14, 27, 26, 25}; // Pins for Sensores in Tank --> Order: 100%, 75%, 50%, 25%, 10%

// EXTERNAL DATA
RTC_DATA_ATTR int plantID = -1;
RTC_DATA_ATTR int thingID = -1;
RTC_DATA_ATTR int bootCount = 0;
RTC_DATA_ATTR int sleepDuration = 24;        // sleep time in minutes
RTC_DATA_ATTR int soilMoistureCritical = 25; // critical soil moisture in % that initiates watering process
RTC_DATA_ATTR int waterAmount = 20;
RTC_DATA_ATTR int pumpPowerMin = 150;
RTC_DATA_ATTR int pumpPowerMax = 230;
RTC_DATA_ATTR int waterFlowCalibration = 250;
RTC_DATA_ATTR int soilMoistureCalibrationAir = 3180;
RTC_DATA_ATTR int soilMoistureCalibrationWater = 1320;
RTC_DATA_ATTR int waterLevelSensorThreshold = 400;

//Flag values (boolean) that are received from the Blynk app --> get reset after reboot or deepsleep
int pumpPowerMinCalibrationFlag = 0;
int pumpPowerMaxCalibrationFlag = 0;
int waterFlowCalibrationFlag = 0;
int soilMoistureCalibrationAirFlag = 0;
int soilMoistureCalibrationWaterFlag = 0;
int waterLevelSensorRawReadingsFlag = 0;

// GLOBALS
// water
const int sensorMeasureWaitingTime = 200; // 200ms between measurements --> same for all measurement functions
int waterLevelSensorReading[5];
int waterLevelPercentage = 0;
const int waterLevelAssociated[] = {100, 75, 50, 25, 10, 0}; // water levels in % associated with each pin
// battery
int batteryLevelReading[10];
int batteryLevelAverage = 0;
float batteryLevelVoltage = 0;
int batteryLevelPercentage = 0;
// soil
int soilMoistureReading[10];
int soilMoistureAverage = 0;
int soilMoistureCalibrated = 0;
int soilMoisturePercentage = 0;
// String uniqueID = ESP.getChipId();
int pumpActivityFlag = 0;

void tryConnectWiFi()
{
  Serial.println(F("Try to connect WiFi"));
  String receivedData;
  receivedData = readStringFromEEPROM(wifiAddr);
  Serial.println(F("Getting wifi credentials from EEPROM"));

  if (receivedData.length() > 0)
  {
    String wifiName = getValue(receivedData, ',', 0);
    String wifiPassword = getValue(receivedData, ',', 1);

    if (wifiName.length() > 0 && wifiPassword.length() > 0)
    {
      hasWiFiCredentials = true;
      Serial.print(F("WifiName : "));
      Serial.println(wifiName);

      Serial.print(F("wifiPassword : "));
      Serial.println(wifiPassword);

      WiFi.begin(wifiName.c_str(), wifiPassword.c_str());

      Serial.print(F("Connecting to Wifi"));
      while (WiFi.status() != WL_CONNECTED)
      {
        isWiFiConnected = true;
        Serial.print(".");
        delay(300);
      }
      Serial.println();
      Serial.print(F("Connected with IP: "));
      Serial.println(WiFi.localIP());
    }
    else
    {
      Serial.println(F("WiFi credentials not valid"));
    }
  }
  else
  {
    Serial.println(F("No WiFi credentials in EEPROM"));
  }
}

class MyServerCallbacks : public BLEServerCallbacks
{
    void onConnect(BLEServer *pServer)
    {
      deviceConnected = true;
      BLEDevice::startAdvertising();
    };

    void onDisconnect(BLEServer *pServer)
    {
      deviceConnected = false;
    }
};

class MyCallbacks : public BLECharacteristicCallbacks
{
    void onWrite(BLECharacteristic *pCharacteristic)
    {
      std::string value = pCharacteristic->getValue();

      if (value.length() > 0)
      {
        Serial.print(F("Value : "));
        Serial.println(value.c_str());
        writeString(wifiAddr, value.c_str());
        tryConnectWiFi();
      }
    }

    void writeString(int add, String data)
    {
      int _size = data.length();
      for (int i = 0; i < _size; i++)
      {
        EEPROM.write(add + i, data[i]);
      }
      EEPROM.write(add + _size, '\0');
      EEPROM.commit();
    }
};

void printDeviceBanner()
{
  Serial.println("--------------------------");
  //Serial.println(String("Product:  ") + BLYNK_DEVICE_NAME);
  //Serial.println(String("Hardware: ") + BOARD_HARDWARE_VERSION);
  //Serial.println(String("Firmware: ") + BLYNK_FIRMWARE_VERSION " (build " __DATE__ " " __TIME__ ")");
  //if (configStore.getFlag(CONFIG_FLAG_VALID)) {
  //  Serial.println(String("Token:    ...") + (configStore.cloudToken+28));
  //}
  Serial.println(String("Frequency:   ") + ESP.getCpuFreqMHz() + "MHz");
  //Serial.println(String("MAC:      ") + WiFi.macAddress());
  Serial.println(String("Flash:    ") + ESP.getFlashChipSize() / 1024 + "K");
  Serial.println(String("ESP sdk:  ") + ESP.getSdkVersion());
  Serial.println(String("Chip rev: ") + ESP.getChipRevision());
  Serial.println(String("Free mem: ") + ESP.getFreeHeap());
  Serial.println("--------------------------");
}

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);
  if (!EEPROM.begin(EEPROM_SIZE))
  {
    delay(1000);
  }
  // for testing purpose
  for (int i = 0; i < EEPROM_SIZE; i++)
  {
    EEPROM.write(i, 0);
  }
  EEPROM.commit();
  delay(500);
  printDeviceBanner();
  tryConnectWiFi();
  if (hasWiFiCredentials && isWiFiConnected)
  {
    digitalWrite(LED_BUILTIN, true);
  }
  else
  {
    bleTask();
  }
}

void bleTask()
{
  // Create the BLE Device
  Serial.println(F("Setting up ESP32 THAT PROJECT"));
  BLEDevice::init("ESP32 THAT PROJECT");

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ |
                      BLECharacteristic::PROPERTY_WRITE |
                      BLECharacteristic::PROPERTY_NOTIFY |
                      BLECharacteristic::PROPERTY_INDICATE);

  pCharacteristic->setCallbacks(new MyCallbacks());
  // https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.descriptor.gatt.client_characteristic_configuration.xml
  // Create a BLE Descriptor
  pCharacteristic->addDescriptor(new BLE2902());
  // Start the service
  pService->start();
  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x0); // set value to 0x00 to not advertise this parameter
  BLEDevice::startAdvertising();
  Serial.println(F("Waiting a client connection to notify..."));
}

String readStringFromEEPROM(int add)
{
  char data[100];
  int len = 0;
  unsigned char k;
  k = EEPROM.read(add);
  while (k != '\0' && len < 500)
  {
    k = EEPROM.read(add + len);
    data[len] = k;
    len++;
  }
  data[len] = '\0';
  return String(data);
}

String getValue(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length() - 1;

  for (int i = 0; i <= maxIndex && found <= index; i++)
  {
    if (data.charAt(i) == separator || i == maxIndex)
    {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }
  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}


void batteryLevelMeasure()
{
  // read battery 10 times
  for (int i = 0; i < 10; i++)
  {
    batteryLevelReading[i] = analogRead(batteryLevelPin);
    batteryLevelAverage += batteryLevelReading[i]; // add current sensor reading to average
    delay(sensorMeasureWaitingTime);
  }
  // do avg
  batteryLevelAverage = batteryLevelAverage / 10;
  // conversion of raw reading to voltage ->
  // multiply with 3.3 by default because of 3.3V basic voltage of the ESP32 -->
  // here its multiplyed by 1.33 (=4.39) because of the voltage divider in the circuit and corrected with an additional correction factor => 4.62
  batteryLevelVoltage = batteryLevelAverage / 4096.0 * 4.62;
  // conversion from voltage to percentage by using a fitting function for the charge-voltage-curve of an LiPo battery
  batteryLevelPercentage = 2808.3808 * pow(batteryLevelVoltage, 4) - 43560.9157 * pow(batteryLevelVoltage, 3) + 252848.5888 * pow(batteryLevelVoltage, 2) - 650767.4615 * batteryLevelVoltage + 626532.9;
  if (batteryLevelVoltage > 4.2)
  {
    batteryLevelPercentage = 100;
  }
  if (batteryLevelVoltage < 3.5)
  {
    batteryLevelPercentage = 0;
  }
}

void waterLevelMeasure()
{
  static int waterLevelPinIndex = 0;
  waterLevelPinIndex = 0;
  for (int n = 0; n < 5; n++)
  {
    waterLevelSensorReading[n] = 0;
  }
  waterLevelPercentage = 0;

  // do this up to 5 times (once for each pin) -->
  // Start measuring from top pins and ends at bottom pins,
  // so pins under water are tested less frequently -->
  // less corrosion on these pins
  for (int waterLevelPinIndex = 0; waterLevelPinIndex < 5; waterLevelPinIndex++)
  {
    pinMode(waterLevelPin[waterLevelPinIndex], OUTPUT);    // set specific water level pin under test to output
    digitalWrite(waterLevelPin[waterLevelPinIndex], HIGH); // activate power on specific water level pin under test
    delay(sensorMeasureWaitingTime);
    waterLevelSensorReading[waterLevelPinIndex] = analogRead(waterLevelGroundPin); // read voltage on ground pin -> if ground and the specific pin under test are connected by water in the tank -> high value
    digitalWrite(waterLevelPin[waterLevelPinIndex], LOW);                          // cut off power to pin
    pinMode(waterLevelPin[waterLevelPinIndex], INPUT);                             // set pin to input
    if (waterLevelSensorReading[waterLevelPinIndex] >= waterLevelSensorThreshold && waterLevelPercentage == 0)
    { // if the specific pin under test and ground pin ARE connectet AND current water level hasent been found yet
      waterLevelPercentage = waterLevelAssociated[waterLevelPinIndex]; // this is the current Water level
      break;
    }
    delay(sensorMeasureWaitingTime);
  }
}

void soilMoistureMeasure()
{
  soilMoistureAverage = 0;
  digitalWrite(moistureSensorPowerPin, HIGH); // power up the sensor
  delay(sensorMeasureWaitingTime);
  // read battery 10 times
  for (int i = 0; i < 10; i++)
  {
    soilMoistureReading[i] = analogRead(moistureSensorSignalPin);
    // add current sensor reading to average
    soilMoistureAverage += soilMoistureReading[i];
    delay(sensorMeasureWaitingTime);
  }
  // cut off power to the sensor
  digitalWrite(moistureSensorPowerPin, LOW);
  // divide the sum of the readings by the number of measurement repetitions
  soilMoistureAverage = soilMoistureAverage / 10;
  if (soilMoistureCalibrationAirFlag == 1)
  {
    soilMoistureCalibrationAir = soilMoistureAverage;
  }
  if (soilMoistureCalibrationWaterFlag == 1)
  {
    soilMoistureCalibrationWater = soilMoistureAverage;
  }
  soilMoistureCalibrated = map(soilMoistureAverage, soilMoistureCalibrationWater, soilMoistureCalibrationAir, 1320, 3173);                                        // calibrate new value with map function
  soilMoisturePercentage = (178147020.5 - 52879.727 * soilMoistureCalibrated) / (1 - 428.814 * soilMoistureCalibrated + 0.9414 * pow(soilMoistureCalibrated, 2)); // Fitting function to calculate %-value
  if (soilMoisturePercentage > 100)
  {
    soilMoisturePercentage = 100;
  }
  if (soilMoisturePercentage < 0)
  {
    soilMoisturePercentage = 0;
  }
}

void pumpOperation()
{
  static const int pumpPwmFrequency = 490;            // frequency of the PWM signal in Hertz -> maximum resolution depends on frequency (higher frequency means lower possible resolution -> max freq [in Hz] = 80000000/(2^resolution) )  -> frequency effects noise of the pump
  static const int pumpPwmChannel = 0;                // choose a PWM channel -> There are 16 channels from 0 to 15
  static const int pumpPwmResolution = 8;             // resolution for the duty cyle of the PWM signal in bit, higher resolution means finer adjustment -> duty cycle range from 0 to (2^resolution) - 1) -> e.g. 8 bit => duty cycle between 0 and 255
  static float batteryVoltageCompensationValue;       // compensation value to always power the pump with 3.5V (lowest possible battery voltage), independet of the actual battery voltage
  static int pumpOperationDuration = 10000;           // set default pump duration to 10000 milliseconds (10 seconds)
  static unsigned long pumpOperationTime;             // starting time of the pump operation
  static int pumpPwmDutyCycle;                        // duty cycle of the PWM signal -> controls power of the pump
  static int pumpPwmDutyCycleMin;                     // starting duty cycle
  static int pumpPwmDutyCycleMax;                     // ending duty cycle
  static int dutyCycleRestTime;                       // time after that duty cycle is increased by 1
  static unsigned long previousDutyCycleIncreaseTime; // last time the duty cycle was increased

  if (soilMoisturePercentage <= soilMoistureCritical || pumpPowerMinCalibrationFlag == 1 || pumpPowerMaxCalibrationFlag == 1 || waterFlowCalibrationFlag == 1)
  { // check if pump needs to be started
    if (waterLevelPercentage < 10)
    { // if the water level is less then 10%
      Serial.println("Water level is too low - minimum water level for pump operation is 10%");
      return;
    }
    if (batteryLevelPercentage < 10)
    { // if the battery level is less then 10%
      Serial.println("Battery level is too low - minimum battery level for pump operation is 10%");
      return;
    }
    if (waterLevelPercentage >= 10 && batteryLevelPercentage >= 10)
    {
      // configure the PWM signal functionalitites for controling the pump
      ledcSetup(pumpPwmChannel, pumpPwmFrequency, pumpPwmResolution);
      // attach a channel to one Pin for generating the PWM signal
      // for controling the pump
      ledcAttachPin(pumpPowerPin, pumpPwmChannel);
      // calculate compensation value to always power the pump with 3.5V
      // (lowest possible battery voltage), independet of the actual battery voltage
      batteryVoltageCompensationValue = -0.238 * batteryLevelVoltage + 1.833;
      if (pumpPowerMinCalibrationFlag == 1)
      {
        // dont change pump PWM while calibrating pump
        // (min and max are almost the same -> +1 to avoid dividing by zero later)
        pumpPwmDutyCycleMin = pumpPowerMin * batteryVoltageCompensationValue;
        pumpPwmDutyCycleMax = pumpPowerMin * batteryVoltageCompensationValue + 1;
      }
      else if (pumpPowerMaxCalibrationFlag == 1)
      {
        // dont change pump PWM while calibrating pump
        // (min and max are almost the same -> +1 to avoid dividing by zero later)
        pumpPwmDutyCycleMin = pumpPowerMax * batteryVoltageCompensationValue;
        pumpPwmDutyCycleMax = pumpPowerMax * batteryVoltageCompensationValue + 1;
      }
      else
      { // increase pump PWM during operation
        pumpPwmDutyCycleMin = pumpPowerMin * batteryVoltageCompensationValue;
        pumpPwmDutyCycleMax = pumpPowerMax * batteryVoltageCompensationValue;
        if (waterFlowCalibrationFlag == 1)
        {
          // set pump duration to 60000 milliseconds (60 seconds)
          pumpOperationDuration = 60000;
        }
        else
        {
          // calculate how long the pump has to operate to pump the desired amount of water
          // --> mulitply by 60000 to get result in milliseconds
          pumpOperationDuration = 60000 * waterAmount / waterFlowCalibration;
          Serial.println("Amount of water to pump: " + String(waterAmount) + " mL");
        }
      }
      dutyCycleRestTime = pumpOperationDuration / (pumpPwmDutyCycleMax - pumpPwmDutyCycleMin); // calculate after how many milliseconds the duty cycle has to be increased in steps of 1, so that it reaches the maximum duty cycle at the end of the pump duration
      Serial.println("Pumping duration: " + String(pumpOperationDuration) + " milliseconds");
      Serial.println("Starting pump PWM duty cycle at: " + String(pumpPwmDutyCycleMin));
      Serial.println("Ending pump PWM duty cycle at: " + String(pumpPwmDutyCycleMax));
      Serial.println("Increasing PWM duty cycle by 1 every: " + String(dutyCycleRestTime) + " milliseconds");
      pumpPwmDutyCycle = pumpPwmDutyCycleMin;   // set duty cycle to the starting value
      pumpOperationTime = millis();             // reset the pump timer to current time
      previousDutyCycleIncreaseTime = millis(); // reset the duty cycle increase timer to current time
      pumpActivityFlag = 1;
      Serial.println("Starting water pump... (Push the hardware button to cancel)");
      while (
        millis() - pumpOperationTime < pumpOperationDuration ||
        digitalRead(buttonPin) == HIGH)
      {
        ledcWrite(pumpPwmChannel, pumpPwmDutyCycle); // send PWM signal with current duty cycle to operate the pump
        if (pumpPwmDutyCycle < pumpPwmDutyCycleMax && millis() - previousDutyCycleIncreaseTime >= dutyCycleRestTime)
        { // if the maximum duty cycle hasnt been reached and its time for a new increase...
          previousDutyCycleIncreaseTime = millis(); // reset the duty cyle increase timer to current time
          Serial.println("Current pump PWM duty cycle: " + String(pumpPwmDutyCycle));
          pumpPwmDutyCycle++; // slowly increase the duty cycle
        }
      }
      ledcWrite(pumpPwmChannel, 0); // turn off the pump by sending a PWM signal with a duty cycle of zero
    }
  }
  else
  {
    Serial.println("No need to start water pump");
  }
}

String httpGETRequest(String url) {
  WiFiClient client;
  HTTPClient http;

  // Your Domain name with URL path or IP address with path
  http.begin(client, url);

  // Send HTTP POST request
  int httpResponseCode = http.GET();

  String payload = "{}";

  if (httpResponseCode > 0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    payload = http.getString();
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  // Free resources
  http.end();

  return payload;
}

void sendDataToServer() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String serverPath = serverHostName + "/vaso/measures?water=" + String(waterLevelPercentage) + "&battery=" + String(batteryLevelPercentage) + "&moist=" + String(soilMoisturePercentage);// + "&id=" + uniqueID;
    // Your Domain name with URL path or IP address with path
    httpGETRequest(serverPath);
  }
  else {
    Serial.println("WiFi Disconnected");
  }
}


void checkOTA() {}

void downloadThresholds() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String serverPath = serverHostName + "/vaso/sync/plant/" + plantID;
    // Your Domain name with URL path or IP address with path
    String newThresholds = httpGETRequest(serverPath);

    Serial.println(newThresholds);
    JSONVar payload = JSON.parse(newThresholds);

    // JSON.typeof(jsonVar) can be used to get the type of the var
    if (JSON.typeof(payload) == "payload") {
      Serial.println("Parsing input failed!");
      return;
    }

    Serial.print("JSON object = ");
    Serial.println(payload);

    if (payload.hasOwnProperty("soilMoistureCritical")) {
      soilMoistureCritical = (int) payload["soilMoistureCritical"];
    } else if (payload.hasOwnProperty("pumpPowerMin")) {
      pumpPowerMin = (int) payload["pumpPowerMin"];
    } else if (payload.hasOwnProperty("pumpPowerMax")) {
      pumpPowerMax = (int) payload["pumpPowerMax"];
    } else if (payload.hasOwnProperty("waterFlowCalibration")) {
      waterFlowCalibration = (int) payload["waterFlowCalibration"];
    } else if (payload.hasOwnProperty("soilMoistureCalibrationAir")) {
      soilMoistureCalibrationAir = (int) payload["soilMoistureCalibrationAir"];
    } else if (payload.hasOwnProperty("soilMoistureCalibrationWater")) {
      soilMoistureCalibrationWater = (int) payload["soilMoistureCalibrationWater"];
    } else if (payload.hasOwnProperty("waterLevelSensorThreshold")) {
      waterLevelSensorThreshold = (int) payload["waterLevelSensorThreshold"];
    } else if (payload.hasOwnProperty("plantID")) {
      plantID = (int) payload["plantID"];
    } else if (payload.hasOwnProperty("thingID")) {
      thingID = (int) payload["thingID"];
    }

  }
  else {
    Serial.println("WiFi Disconnected");
  }
}

void loop()
{
  if (isWiFiConnected)
  {
    checkOTA();
    downloadThresholds();
    batteryLevelMeasure();
    waterLevelMeasure();
    soilMoistureMeasure();
    pumpOperation();
    sendDataToServer();
    Serial.println(F("Try to update thresholds values from the server"));
    Serial.println(F("Read data from sensors and send it to the server"));
  }
  else
  {
    Serial.println(F("No connection, doing nothing"));
  }
  delay(15000);
  // put your main code here, to run repeatedly:
}
