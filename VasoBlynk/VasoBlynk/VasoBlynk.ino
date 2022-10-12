// Fill-in information from your Blynk Template here
#define BLYNK_TEMPLATE_ID "TMPLt08Ysl0r"
#define BLYNK_DEVICE_NAME "Vaso"

#define BLYNK_FIRMWARE_VERSION "0.1.3"

#define BLYNK_PRINT Serial
//#define BLYNK_DEBUG

#define APP_DEBUG

// Uncomment your board, or configure a custom board in Settings.h
//#define USE_WROVER_BOARD

#include "BlynkEdgent.h"
#include <driver/adc.h>

// Global variables for unit conversion
#define hoursToSeconds 3600LL           // conversion factor from hours to seconds --> suffix LL for definition as 64bit constant
#define secondsToMikroseconds 1000000LL // conversion factor from seconds to mikroseconds --> suffix LL for definition as 64bit constant

// PIN Configuration
// Conflict with analog ADC2 Pins and Wifi --> they stop working forever after wifi is startet --> only use ADC1 Pins (32,33,34,35,36,39) for analogread
const byte buttonPin = 0;        // If changed, the pin also has to be changed in the Sleep function
const byte batteryLevelPin = 32; // voltage divider with 1MOhm and 30KOhm Resistor used to reduce maximum volatge at this pin (3.4V to 3.33V) --> at minimum operating voltage of 2.55V(???) readings at this pin would be 2,47V
const byte pumpPowerPin = 23;
const byte moistureSensorPowerPin = 19;
const byte moistureSensorSignalPin = 33;
const byte waterLevelGroundPin = 35;               // lowest cable in tank, connect 100kohm Resistor to Ground parallel as pulldown resitor
const byte waterLevelPin[] = {13, 14, 27, 26, 25}; // Pins for Sensores in Tank --> Order: 100%, 75%, 50%, 25%, 10%

// Config values --> can be changed in the Blynk app
// RTC Memory values --> these are preserved during deepsleep but lost when reseting or powering off the esp32
RTC_DATA_ATTR int bootCount = 0;
RTC_DATA_ATTR int mode = 0;    // 0 = auto, 1 = vacation
RTC_DATA_ATTR int daysOff = 0; // number of days on which the water must be rationated
RTC_DATA_ATTR int vacationStartAt = 0;
RTC_DATA_ATTR int sleepDuration = 10;        // sleep time in minutes
RTC_DATA_ATTR int soilMoistureCritical = 25; // critical soil moisture in % that initiates watering process
RTC_DATA_ATTR int waterAmount = 20;
RTC_DATA_ATTR int pumpPowerMin = 150;
RTC_DATA_ATTR int pumpPowerMax = 230;
RTC_DATA_ATTR int waterFlowCalibration = 250;
RTC_DATA_ATTR int soilMoistureCalibrationAir = 3180;
RTC_DATA_ATTR int soilMoistureCalibrationWater = 1320;
RTC_DATA_ATTR int waterLevelSensorThreshold = 400;

// Flag values (boolean) that are received from the Blynk app --> get reset after reboot or deepsleep
int pumpPowerMinCalibrationFlag = 0;
int pumpPowerMaxCalibrationFlag = 0;
int waterFlowCalibrationFlag = 0;
int soilMoistureCalibrationAirFlag = 0;
int soilMoistureCalibrationWaterFlag = 0;
int waterLevelSensorRawReadingsFlag = 0;

// Global variables for state machine operation
byte downloadBlynkState = 0;
byte uploadBlynkState = 0;
byte batteryLevelMeasureState = 0;
byte waterLevelMeasureState = 0;
byte soilMoistureMeasureState = 0;
byte pumpOperationState = 0;
byte routineState = 0;

// Global variables (Blynk related)
int blynkSyncCounter = 0;
int blynkSyncNumber = 17;          // number of values that have to be downloaded from Blynk server during synchronisation
boolean blynkSyncRequired = false; // flag if sync with Blynk server is required
boolean BlynkInitialized = false;

// Other global variables
esp_sleep_wakeup_cause_t deepsleepWakeupReason;
const int sensorMeasureWaitingTime = 200; // 200ms between measurements --> same for all measurement functions
int batteryLevelReading[10];
int batteryLevelAverage = 0;
float batteryLevelVoltage = 0;
int batteryLevelPercentage = 0;
int waterLevelSensorReading[5];
int waterLevelPercentage = 0;
const int waterLevelAssociated[] = {100, 75, 50, 25, 10, 0}; // water levels in % associated with each pin
int soilMoistureReading[10];
int soilMoistureAverage = 0;
int soilMoistureCalibrated = 0;
int soilMoisturePercentage = 0;
int pumpActivityFlag = 0;

void setup()
{
  delay(500); // delay here is required to wake up reliably
  pinMode(buttonPin, INPUT);
  pinMode(batteryLevelPin, INPUT);
  pinMode(pumpPowerPin, OUTPUT);
  pinMode(moistureSensorSignalPin, INPUT);
  pinMode(moistureSensorPowerPin, OUTPUT);
  pinMode(waterLevelGroundPin, INPUT); // cathode --> only here no corrosion
  for (int i = 0; i < 5; i++)
  {
    pinMode(waterLevelPin[i], INPUT); // corrosion on these anodic pins
  }
  pinMode(LED_BUILTIN, OUTPUT);
  // digitalWrite(LED_BUILTIN, LOW);   // turn the LED ON (strangely Builddin LED is active LOW) --> turns off when entering deepsleep
  Serial.begin(115200);
  delay(100); // wait for serial monitor to open
  bootCount++;

  timer.setTimeout(120000L, DeepSleep); // Setup DeepSleep function to be called after 120000 second --> go to deep sleep if anything takes to long
  routineState = 1;
  BLYNK_LOG("Bootcount: %i", bootCount);
}

void routine()
{
  switch (routineState)
  {
  case 1: // download new config values from Blynk if sync is required
    downloadBlynkState = 1;
    routineState++;
    break;
  case 2: // start measuring processes
    if (downloadBlynkState == 100)
    {
      batteryLevelMeasureState = 1;
      waterLevelMeasureState = 1;
      soilMoistureMeasureState = 1;
      routineState++;
    }
    break;
  case 3:
    if (batteryLevelMeasureState == 100 && waterLevelMeasureState == 100 && soilMoistureMeasureState == 100)
    {
      serialPrintValues();    // print measurement values to serial monitor
      pumpOperationState = 1; // start the pump
      routineState++;
    }
    break;
  case 4:
    if (pumpOperationState == 100)
    {
      uploadBlynkState = 1; // upload new values
      routineState++;
    }
    break;
  case 5:
    if (uploadBlynkState == 100)
    {
      DeepSleep();
    }
    break;
  }
}

void serialPrintValues()
{
  // for (int n = 0; n < 10; n++)
  //{
  //   BLYNK_LOG("Raw battery level reading %i: %i", n + 1, batteryLevelReading[n]);
  // }
  BLYNK_LOG("Average battery level reading: %i", batteryLevelAverage);
  BLYNK_LOG("Battery level voltage: %f V", batteryLevelVoltage);
  BLYNK_LOG("Battery level percentage: %i %%", batteryLevelPercentage);
  if (waterLevelSensorRawReadingsFlag == 1)
  {
    for (int n = 0; n < 5; n++)
    {
      BLYNK_LOG("Raw reading for pin on water level %i: %i", waterLevelAssociated[n], waterLevelSensorReading[n]);
    }
  }
  BLYNK_LOG("Water level percentage: %i %%", waterLevelPercentage);
  //  for (int n = 0; n < 10; n++)
  //  {
  //    BLYNK_LOG("Raw soil moisture reading %i: %i", n + 1, soilMoistureReading[n]);
  //  }
  BLYNK_LOG("Average soil moisture reading: %i", soilMoistureAverage);
  BLYNK_LOG("Calibrated soil moisture reading: %i", soilMoistureCalibrated);
  BLYNK_LOG("Soil moisture percentage: %i", soilMoisturePercentage);
}

void DeepSleep()
{
  Blynk.disconnect();
  WiFi.mode(WIFI_OFF);                                                       // this is needed to reduce power consumption during deep sleep if there is a ext0 wakeup source defined (ext1 would work without this line)
  adc_power_off();                                                           // this is needed to reduce power consumption during deep sleep if there is a ext0 wakeup source defined (ext1 would work without this line)
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_0, LOW);                             // wakup after timer runs out or pin gets pulled low by pressing the wakeup-button (GPI0_NUM_PinNumber has to be written this way)
  esp_sleep_enable_timer_wakeup(sleepDuration * secondsToMikroseconds * 60); // set sleep timer (in minutes)
  BLYNK_LOG("Going to sleep for %i minuts or until hardware button is pressed...", sleepDuration);
  esp_deep_sleep_start(); // start deepsleep
}

void loop()
{
  downloadBlynk();
  uploadBlynk();
  batteryLevelMeasure();
  waterLevelMeasure();
  soilMoistureMeasure();
  pumpOperation();
  routine();
  if (BlynkInitialized == true)
  {
    BlynkEdgent.run();
  }
  timer.run(); // Initiates BlynkTimer
}
