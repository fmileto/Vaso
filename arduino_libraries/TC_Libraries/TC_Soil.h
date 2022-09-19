#include "Arduino.h"

/* Uncomment to enable printing out nice debug messages. */
//#define SOILSENSOR_DEBUG

#define DEBUG_PRINTER                                      \
    Serial /**< Define where debug output will be printed. \
            */

/* Setup debug printing macros. */
#ifdef SOILSENSOR_DEBUG
#define DEBUG_PRINT(...)                  \
    {                                     \
        DEBUG_PRINTER.print(__VA_ARGS__); \
    }
#define DEBUG_PRINTLN(...)                  \
    {                                       \
        DEBUG_PRINTER.println(__VA_ARGS__); \
    }
#else
#define DEBUG_PRINT(...) \
    {                    \
    } /**< Debug Print Placeholder if Debug is disabled */
#define DEBUG_PRINTLN(...) \
    {                      \
    } /**< Debug Print Line Placeholder if Debug is disabled */
#endif

class SOILSENSOR
{
public:
    SOILSENSOR(uint8_t pin);
    void begin();
    int readHumidityPercent();

private:
    uint8_t _pin;
    int _sensorMeasureWaitingTime;
    int _soilMoistureAverage;
    int _soilMoistureCalibrated;
    int _soilMoisturePercentage;
    int _soilMoistureReading[10];
    int _soilMoistureCalibrationAir;
    int _soilMoistureCalibrationWater;
}