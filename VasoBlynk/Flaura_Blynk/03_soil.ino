
void soilMoistureMeasure()
{
    // BLYNK_LOG("SOIL TASK state: %i", soilMoistureMeasureState);
    static int soilMoistureReadingCounter = 0;
    static unsigned long previousSoilMoistureSensorMeasureTime = 0;
    switch (soilMoistureMeasureState)
    {
    case 1:                                         // do this on first run of this task -> reset values
        digitalWrite(moistureSensorPowerPin, HIGH); // power up the sensor
        soilMoistureReadingCounter = 0;
        soilMoistureAverage = 0;
        previousSoilMoistureSensorMeasureTime = millis(); // reset measurement timer -> to wait a short time before measuring for the voltage on the sensor to rise
        soilMoistureMeasureState++;
        break;
    case 2:
        if (soilMoistureReadingCounter < 10 && millis() - previousSoilMoistureSensorMeasureTime >= sensorMeasureWaitingTime)
        {                                                     // do 10 measurement repetitions every few milliseconds
            previousSoilMoistureSensorMeasureTime = millis(); // reset measure timer
            soilMoistureReading[soilMoistureReadingCounter] = analogRead(moistureSensorSignalPin);
            soilMoistureAverage += soilMoistureReading[soilMoistureReadingCounter]; // add current sensor reading to average
            soilMoistureReadingCounter++;
        }
        if (soilMoistureReadingCounter == 10)
        {
            soilMoistureMeasureState++;
        }
        break;
    case 3:                                             // after all measurement repetitions have been performed
        digitalWrite(moistureSensorPowerPin, LOW);      // cut off power to the sensor
        soilMoistureAverage = soilMoistureAverage / 10; // divide the sum of the readings by the number of measurement repetitions
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
        soilMoistureMeasureState = 100; // mark task as finsihed
        break;
    }
}