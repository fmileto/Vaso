
void batteryLevelMeasure()
{
    // BLYNK_LOG("BATTERY TASK state: %i", batteryLevelMeasureState);
    static int batteryReadingCounter = 0;
    static unsigned long previousBatteryLevelMeasureTime = 0;
    switch (batteryLevelMeasureState)
    {
    case 1: // do this on first run of this task -> reset values
        batteryLevelAverage = 0;
        batteryReadingCounter = 0;
        batteryLevelMeasureState++;
        break;
    case 2:
        if (batteryReadingCounter < 10 && millis() - previousBatteryLevelMeasureTime >= sensorMeasureWaitingTime)
        {                                               // do 10 measurement repetitions every few milliseconds
            previousBatteryLevelMeasureTime = millis(); // reset measure timer
            batteryLevelReading[batteryReadingCounter] = analogRead(batteryLevelPin);
            batteryLevelAverage += batteryLevelReading[batteryReadingCounter]; // add current sensor reading to average
            batteryReadingCounter++;
        }
        if (batteryReadingCounter == 10)
        {
            batteryLevelMeasureState++;
        }
        break;
    case 3: // after all measurement repetitions have been performed
        batteryLevelAverage = batteryLevelAverage / 10;
        batteryLevelVoltage = batteryLevelAverage / 4096.0 * 4.62;                                                                                                                                              // conversion of raw reading to voltage -> multiply with 3.3 by default because of 3.3V basic voltage of the ESP32 --> here its multiplyed by 1.33 (=4.39) because of the voltage divider in the circuit and corrected with an additional correction factor => 4.62
        batteryLevelPercentage = 2808.3808 * pow(batteryLevelVoltage, 4) - 43560.9157 * pow(batteryLevelVoltage, 3) + 252848.5888 * pow(batteryLevelVoltage, 2) - 650767.4615 * batteryLevelVoltage + 626532.9; // conversion from voltage to percentage by using a fitting function for the charge-voltage-curve of an LiPo battery
        if (batteryLevelVoltage > 4.2)
        {
            batteryLevelPercentage = 100;
        }
        if (batteryLevelVoltage < 3.5)
        {
            batteryLevelPercentage = 0;
        }
        batteryLevelMeasureState = 100; // mark task as finsihed
        break;
    }
}