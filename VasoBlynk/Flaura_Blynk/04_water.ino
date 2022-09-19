
void waterLevelMeasure()
{
    //   BLYNK_LOG("WATER TASK state: %i", waterLevelMeasureState);
    static int waterLevelPinIndex = 0;
    static unsigned long previousWaterLevelMeasureTime = 0;
    switch (waterLevelMeasureState)
    {
    case 1: // do this on first run of this task -> reset values
        waterLevelPinIndex = 0;
        for (int n = 0; n < 5; n++)
        {
            waterLevelSensorReading[n] = 0;
        }
        waterLevelPercentage = 0;
        waterLevelMeasureState++;
        break;
    case 2:
        if (waterLevelPinIndex < 5)
        {                                                          // do this up to 5 times (once for each pin) --> Start measuring from top pins and ends at bottom pins, so pins under water are tested less frequently --> less corrosion on these pins
            pinMode(waterLevelPin[waterLevelPinIndex], OUTPUT);    // set specific water level pin under test to output
            digitalWrite(waterLevelPin[waterLevelPinIndex], HIGH); // activate power on specific water level pin under test
            if (millis() - previousWaterLevelMeasureTime >= sensorMeasureWaitingTime)
            {                                                                                  // wait for the voltage on the pin to rise
                previousWaterLevelMeasureTime = millis();                                      // reset timer
                waterLevelSensorReading[waterLevelPinIndex] = analogRead(waterLevelGroundPin); // read voltage on ground pin -> if ground and the specific pin under test are connected by water in the tank -> high value
                digitalWrite(waterLevelPin[waterLevelPinIndex], LOW);                          // cut off power to pin
                pinMode(waterLevelPin[waterLevelPinIndex], INPUT);                             // set pin to input
                if (waterLevelSensorReading[waterLevelPinIndex] >= waterLevelSensorThreshold && waterLevelPercentage == 0)
                {                                                                    // if the specific pin under test and ground pin ARE connectet AND current water level hasent been found yet
                    waterLevelPercentage = waterLevelAssociated[waterLevelPinIndex]; // this is the current Water level
                }
                if (waterLevelSensorRawReadingsFlag == 0 && waterLevelPercentage != 0)
                {                                 // if raw values should NOT be printed AND current water level has been found
                    waterLevelMeasureState = 100; // mark task as finished
                }
                waterLevelPinIndex++; // check next pin
            }
        }
        else
        {
            waterLevelMeasureState = 100; // mark task as finished
        }
        break;
    }
}
