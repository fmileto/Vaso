#include "OTA.h"

void setup() {
  Serial.begin(115200);
  Serial.println("Booting");
  ArduinoOTA.setHostname("TemplateSketch");
  setupOTA("TemplateSketch", "The Connective", "th3KK0nn4ctiv3780!");

}

void loop() {
  ArduinoOTA.handle();
}
