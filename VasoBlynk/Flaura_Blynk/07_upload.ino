
void uploadBlynk()
{
    //  BLYNK_LOG("UPLOAD TASK state: %i", uploadBlynkState);
    static unsigned long previousUploadCheckTime = 0;
    switch (uploadBlynkState)
    {
    case 1:
        BLYNK_LOG("Uploading new values to Blynk...");
        BlynkEdgent.begin();
        BlynkInitialized = true;
        uploadBlynkState++;
        break;
    case 2:
        if (millis() - previousUploadCheckTime >= 3000)
        { // check every 3 seconds if boot count has been uploaded yet
            previousUploadCheckTime = millis();
            Blynk.syncVirtual(V104); // needed to trigger BLYNK_WRITE(V104)
        }
        break;
    }
}