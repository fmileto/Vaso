void pumpOperation()
{
    // BLYNK_LOG("PUMP TASK state: %i", pumpOperationState);
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
    switch (pumpOperationState)
    {
    case 1:
        if (soilMoisturePercentage <= soilMoistureCritical || pumpPowerMinCalibrationFlag == 1 || pumpPowerMaxCalibrationFlag == 1 || waterFlowCalibrationFlag == 1)
        { // check if pump needs to be started
            if (waterLevelPercentage < 10)
            { // if the water level is less then 10%
                BLYNK_LOG("Water level is too low - minimum water level for pump operation is 10%");
                pumpOperationState = 100; // mark task as finished
            }
            if (batteryLevelPercentage < 10)
            { // if the battery level is less then 10%
                BLYNK_LOG("Battery level is too low - minimum battery level for pump operation is 10%");
                pumpOperationState = 100; // mark task as finished
            }
            if (waterLevelPercentage >= 10 && batteryLevelPercentage >= 10)
            {
                pumpOperationState++;
            }
        }
        else
        {
            BLYNK_LOG("No need to start water pump");
            pumpOperationState = 100; // mark task as finished
        }
        break;
    case 2:
        ledcSetup(pumpPwmChannel, pumpPwmFrequency, pumpPwmResolution);         // configure the PWM signal functionalitites for controling the pump
        ledcAttachPin(pumpPowerPin, pumpPwmChannel);                            // attach a channel to one Pin for generating the PWM signal for controling the pump
        batteryVoltageCompensationValue = -0.238 * batteryLevelVoltage + 1.833; // calculate compensation value to always power the pump with 3.5V (lowest possible battery voltage), independet of the actual battery voltage
        if (pumpPowerMinCalibrationFlag == 1)
        { // dont change pump PWM while calibrating pump (min and max are almost the same -> +1 to avoid dividing by zero later)
            pumpPwmDutyCycleMin = pumpPowerMin * batteryVoltageCompensationValue;
            pumpPwmDutyCycleMax = pumpPowerMin * batteryVoltageCompensationValue + 1;
        }
        else if (pumpPowerMaxCalibrationFlag == 1)
        { // dont change pump PWM while calibrating pump (min and max are almost the same -> +1 to avoid dividing by zero later)
            pumpPwmDutyCycleMin = pumpPowerMax * batteryVoltageCompensationValue;
            pumpPwmDutyCycleMax = pumpPowerMax * batteryVoltageCompensationValue + 1;
        }
        else
        { // increase pump PWM during operation
            pumpPwmDutyCycleMin = pumpPowerMin * batteryVoltageCompensationValue;
            pumpPwmDutyCycleMax = pumpPowerMax * batteryVoltageCompensationValue;
            if (waterFlowCalibrationFlag == 1)
            {
                pumpOperationDuration = 60000; // set pump duration to 60000 milliseconds (60 seconds)
            }
            else
            {
                pumpOperationDuration = 60000 * waterAmount / waterFlowCalibration; // calculate how long the pump has to operate to pump the desired amount of water  --> mulitply by 60000 to get result in milliseconds
                BLYNK_LOG("Amount of water to pump: %i mL", waterAmount);
            }
        }
        dutyCycleRestTime = pumpOperationDuration / (pumpPwmDutyCycleMax - pumpPwmDutyCycleMin); // calculate after how many milliseconds the duty cycle has to be increased in steps of 1, so that it reaches the maximum duty cycle at the end of the pump duration
        BLYNK_LOG("Pumping duration: %i milliseconds", pumpOperationDuration);
        BLYNK_LOG("Starting pump PWM duty cycle at: %i", pumpPwmDutyCycleMin);
        BLYNK_LOG("Ending pump PWM duty cycle at: %i", pumpPwmDutyCycleMax);
        BLYNK_LOG("Increasing PWM duty cycle by 1 every: %i milliseconds", dutyCycleRestTime);
        pumpPwmDutyCycle = pumpPwmDutyCycleMin;   // set duty cycle to the starting value
        pumpOperationTime = millis();             // reset the pump timer to current time
        previousDutyCycleIncreaseTime = millis(); // reset the duty cycle increase timer to current time
        pumpActivityFlag = 1;
        BLYNK_LOG("Starting water pump... (Push the hardware button to cancel)");
        pumpOperationState++;
        break;
    case 3:
        if (millis() - pumpOperationTime < pumpOperationDuration)
        {                                                // as long as the pump duration has not been reached...
            ledcWrite(pumpPwmChannel, pumpPwmDutyCycle); // send PWM signal with current duty cycle to operate the pump
            if (pumpPwmDutyCycle < pumpPwmDutyCycleMax && millis() - previousDutyCycleIncreaseTime >= dutyCycleRestTime)
            {                                             // if the maximum duty cycle hasnt been reached and its time for a new increase...
                previousDutyCycleIncreaseTime = millis(); // reset the duty cyle increase timer to current time
                // BLYNK_LOG("Current pump PWM duty cycle: %i", pumpPwmDutyCycle);
                pumpPwmDutyCycle++; // slowly increase the duty cycle
            }
        }
        if (millis() - pumpOperationTime >= pumpOperationDuration || digitalRead(buttonPin) == LOW)
        {                                 // after the pump duration has been reached OR if the pump was stopped manually by pushing the button
            ledcWrite(pumpPwmChannel, 0); // turn off the pump by sending a PWM signal with a duty cycle of zero
            BLYNK_LOG("Water pumping has finished or was canceled manually by pushing the hardware button");
            pumpOperationState = 100; // mark task as finished
        }
        break;
    }
}