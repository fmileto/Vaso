
BLYNK_CONNECTED()
{ // gets called as soon as connection to Blynk server is established
  // BLYNK_LOG("CONNECTED, Requires sync: %s", blynkSyncRequired ? "yes" : 'no');
    if (blynkSyncRequired == true)
    {
        BLYNK_LOG("Synchronisation with Blynk server started");
        Blynk.syncVirtual(
            V105,
            V106,
            V107,
            V0,
            V1,
            V2,
            V10,
            V3,
            V4,
            V5,
            V6,
            V7,
            V8,
            V9,
            V11,
            V17,
            V18); // get latest config values from Blynk server
    }
    else
    {
        BLYNK_LOG("Upload of new values to Blynk server started");
        Blynk.virtualWrite(V102, batteryLevelPercentage);
        Blynk.virtualWrite(V19, batteryLevelVoltage);
        Blynk.virtualWrite(V101, waterLevelPercentage);
        if (waterLevelSensorRawReadingsFlag == 1)
        {
            Blynk.virtualWrite(V12, waterLevelSensorReading[0]); // upload raw reading on 100% pin
            Blynk.virtualWrite(V13, waterLevelSensorReading[1]); // upload raw reading on 75% pin
            Blynk.virtualWrite(V14, waterLevelSensorReading[2]); // upload raw reading on 50% pin
            Blynk.virtualWrite(V15, waterLevelSensorReading[3]); // upload raw reading on 25% pin
            Blynk.virtualWrite(V16, waterLevelSensorReading[4]); // upload raw reading on 10% pin
            Blynk.virtualWrite(V11, 0);                          // upload zero to reset flag on server if it was set
        }
        Blynk.virtualWrite(V100, soilMoisturePercentage);
        Blynk.virtualWrite(V20, soilMoistureAverage);
        if (soilMoisturePercentage <= soilMoistureCritical)
        {
            BLYNK_LOG("Sending dry_soil event");
            Blynk.logEvent("dry_soil");
        }
        if (batteryLevelPercentage <= 15)
        {
            BLYNK_LOG("Sending low_battery event");
            Blynk.logEvent("low_battery");
        }
        if (waterLevelPercentage <= 10)
        {
            BLYNK_LOG("Sending low_water event");
            Blynk.logEvent("low_water");
        }
        if (soilMoistureCalibrationAirFlag == 1)
        {
            Blynk.virtualWrite(V5, soilMoistureCalibrationAir);
            Blynk.virtualWrite(V7, 0); // upload zero to reset flag on server if it was set
        }
        if (soilMoistureCalibrationWaterFlag == 1)
        {
            Blynk.virtualWrite(V6, soilMoistureCalibrationWater);
            Blynk.virtualWrite(V8, 0); // upload zero to reset flag on server if it was set
        }
        Blynk.virtualWrite(V103, pumpActivityFlag);
        if (pumpPowerMinCalibrationFlag == 1 || pumpPowerMaxCalibrationFlag == 1 || waterFlowCalibrationFlag == 1)
        { // upload zero to reset flags on server if any of them was set
            Blynk.virtualWrite(V4, 0);
            Blynk.virtualWrite(V2, 0);
            Blynk.virtualWrite(V10, 0);
        }
        Blynk.virtualWrite(V104, bootCount);
    }
}

BLYNK_WRITE(V104)
{                                        // gets called by Blynk.syncVirtual function
    int bootCountServer = param.asInt(); // Get value as int
    if (bootCount == bootCountServer)
    { // if boot count on device and server are matching --> upload has finished
        BLYNK_LOG("Upload of new values to Blynk server confirmed");
        uploadBlynkState = 100; // mark upload task as finsihed
    }
}

BLYNK_WRITE(V105)
{                                  // gets called by Blynk.syncVirtual function
    sleepDuration = param.asInt(); // Get value as int
    BLYNK_LOG("New value for sleep duration received: %i", sleepDuration);
    blynkSyncCounter++;
}

BLYNK_WRITE(V17)
{                            // gets called by Blynk.syncVirtual function
    daysOff = param.asInt(); // Get value as int
    BLYNK_LOG("New value for days off received: %i", daysOff);
    blynkSyncCounter++;
}

BLYNK_WRITE(V18)
{
    if (mode == 0 && param.asInt() == 1)
    {
        // vacationStartAt = millis();
    }
    // gets called by Blynk.syncVirtual function
    mode = param.asInt(); // Get value as int
    BLYNK_LOG("New value for mode received: %i", mode);
    blynkSyncCounter++;
}

BLYNK_WRITE(V106)
{                                         // gets called by Blynk.syncVirtual function
    soilMoistureCritical = param.asInt(); // Get value as int
    BLYNK_LOG("New value for critical soil moisture received: %i", soilMoistureCritical);
    blynkSyncCounter++;
}

BLYNK_WRITE(V107)
{                                // gets called by Blynk.syncVirtual function
    waterAmount = param.asInt(); // Get value as int
    BLYNK_LOG("New value for water amount received: %i", waterAmount);
    blynkSyncCounter++;
}

BLYNK_WRITE(V0)
{                                 // gets called by Blynk.syncVirtual function
    pumpPowerMin = param.asInt(); // Get value as int
    BLYNK_LOG("New value for minimal pump power received: %i", pumpPowerMin);
    blynkSyncCounter++;
}

BLYNK_WRITE(V1)
{                                 // gets called by Blynk.syncVirtual function
    pumpPowerMax = param.asInt(); // Get value as int
    BLYNK_LOG("New value for maximum pump power received: %i", pumpPowerMax);
    blynkSyncCounter++;
}

BLYNK_WRITE(V2)
{                                                // gets called by Blynk.syncVirtual function
    pumpPowerMinCalibrationFlag = param.asInt(); // Get value as int
    BLYNK_LOG("New flag for minimum pump power calibration received: %i", pumpPowerMinCalibrationFlag);
    blynkSyncCounter++;
}

BLYNK_WRITE(V10)
{                                                // gets called by Blynk.syncVirtual function
    pumpPowerMaxCalibrationFlag = param.asInt(); // Get value as int
    BLYNK_LOG("New flag for maximum pump power calibration received: %i", pumpPowerMaxCalibrationFlag);
    blynkSyncCounter++;
}

BLYNK_WRITE(V3)
{                                         // gets called by Blynk.syncVirtual function
    waterFlowCalibration = param.asInt(); // Get value as int
    BLYNK_LOG("New value for water flow calibration received: %i", waterFlowCalibration);
    blynkSyncCounter++;
}

BLYNK_WRITE(V4)
{                                             // gets called by Blynk.syncVirtual function
    waterFlowCalibrationFlag = param.asInt(); // Get value as int
    BLYNK_LOG("New flag for water flow calibration received: %i", waterFlowCalibrationFlag);
    blynkSyncCounter++;
}

BLYNK_WRITE(V5)
{                                               // gets called by Blynk.syncVirtual function
    soilMoistureCalibrationAir = param.asInt(); // Get value as int
    BLYNK_LOG("New value for soil moisture calibration in air received: %i", soilMoistureCalibrationAir);
    blynkSyncCounter++;
}

BLYNK_WRITE(V6)
{                                                 // gets called by Blynk.syncVirtual function
    soilMoistureCalibrationWater = param.asInt(); // Get value as int
    BLYNK_LOG("New value for soil moisture calibration in water received: %i", soilMoistureCalibrationWater);
    blynkSyncCounter++;
}

BLYNK_WRITE(V7)
{                                                   // gets called by Blynk.syncVirtual function
    soilMoistureCalibrationAirFlag = param.asInt(); // Get value as int
    BLYNK_LOG("New flag for soil moisture calibration in air received: %i", soilMoistureCalibrationAirFlag);
    blynkSyncCounter++;
}

BLYNK_WRITE(V8)
{                                                     // gets called by Blynk.syncVirtual function
    soilMoistureCalibrationWaterFlag = param.asInt(); // Get value as int
    BLYNK_LOG("New flag for soil moisture calibration in water received: %i", soilMoistureCalibrationWaterFlag);
    blynkSyncCounter++;
}

BLYNK_WRITE(V9)
{                                              // gets called by Blynk.syncVirtual function
    waterLevelSensorThreshold = param.asInt(); // Get value as int
    BLYNK_LOG("New value for water level sensor threshold received: %i", waterLevelSensorThreshold);
    blynkSyncCounter++;
}

BLYNK_WRITE(V11)
{                                                    // gets called by Blynk.syncVirtual function
    waterLevelSensorRawReadingsFlag = param.asInt(); // Get value as int
    BLYNK_LOG("New flag for raw water level sensor readings received: %i", waterLevelSensorRawReadingsFlag);
    blynkSyncCounter++;
}
