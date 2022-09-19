#include "TC_Soil.h"

SOILSENSOR::SOILSENSOR(uint8_t pin)
{
    _pin = pin;
    _soilMoistureAverage = 0;
    _soilMoistureCalibrated = 0;
    _soilMoisturePercentage = 0;
    _sensorMeasureWaitingTime = 200;
    _soilMoistureReading[10];
    _soilMoistureCalibrationAir = 3180;
    _soilMoistureCalibrationWater = 1320;
}

void SOILSENSOR::begin()
{
    // set up the pins!
    pinMode(_pin, INPUT);
}

/*!
 *  @brief  Read Humidity
 *	@return Humidity in percent value
 */
int SOILSENSOR::readHumidityPercent()
{
    _soilMoistureAverage = 0;
    delay(_sensorMeasureWaitingTime);
    // read battery 10 times
    for (int i = 0; i < 10; i++)
    {
        soilMoistureReading[i] = analogRead(_pin);
        // add current sensor reading to average
        _soilMoistureAverage += soilMoistureReading[i];
        delay(_sensorMeasureWaitingTime);
    }
    // cut off power to the sensor
    // divide the sum of the readings by the number of measurement repetitions
    _soilMoistureAverage = _soilMoistureAverage / 10;

    _soilMoistureCalibrated = map(_soilMoistureAverage, _soilMoistureCalibrationWater, _soilMoistureCalibrationAir, 1320, 3173); // calibrate new value with map function
    _soilMoisturePercentage = (178147020.5 - 52879.727 *
                                                 _soilMoistureCalibrated) /
                              (1 - 428.814 * _soilMoistureCalibrated + 0.9414 * pow(_soilMoistureCalibrated, 2)); // Fitting function to calculate %-value
    if (_soilMoisturePercentage > 100)
    {
        _soilMoisturePercentage = 100;
    }
    if (_soilMoisturePercentage < 0)
    {
        _soilMoisturePercentage = 0;
    }
    DEBUG_PRINT("avg: ");
    DEBUG_PRINTLN(_soilMoistureAverage);
    DEBUG_PRINT("cal: ");
    DEBUG_PRINTLN(_soilMoisturePercentage);
    DEBUG_PRINT("per: ");
    DEBUG_PRINTLN(_soilMoisturePercentage);
}