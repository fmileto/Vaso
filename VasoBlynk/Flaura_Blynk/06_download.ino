
void downloadBlynk()
{
    //   BLYNK_LOG("DOWNLOAD TASK state: %i", downloadBlynkState);
    switch (downloadBlynkState)
    {
    case 1:
        // esp_sleep_wakeup_cause_t deepsleepWakeupReason = esp_sleep_get_wakeup_cause(); //get wakeup reason
        deepsleepWakeupReason = esp_sleep_get_wakeup_cause(); // get wakeup reason
        if (deepsleepWakeupReason == ESP_SLEEP_WAKEUP_EXT0 || bootCount == 1)
        { // if this is the first boot or if button was used to wake up manually
            BLYNK_LOG("This is the first boot of the microcontroller or it was manually woken up from standby mode by button press");
            blynkSyncRequired = true;
            BlynkEdgent.begin();
            BlynkInitialized = true;
            downloadBlynkState++;
        }
        else
        {
            BLYNK_LOG("The microcontroller was regularly woken up from standby mode by the timer - synchronisation with Blynk server is not necassary");
            downloadBlynkState = 100; // mark task as finished
        }
        break;
    case 2:
        if (blynkSyncCounter == blynkSyncNumber)
        { // if all values have been updated
            BLYNK_LOG("Synchronisation finished!");
            blynkSyncRequired = false; // reset flag
            blynkSyncCounter = 0;      // reset counter
            Blynk.disconnect();
            WiFi.disconnect();
            BlynkInitialized = false;
            downloadBlynkState = 100; // mark task as finished
        }
        else
        {
            // BLYNK_LOG("Waiting for synchronisation to complete...");
        }
        break;
    }
}