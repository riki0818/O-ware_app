#include "CoreModule.h"

CoreModule cm;
PHSensor ph(cm, cm.A1);

void setup() {
  Serial.begin(115200);
  cm.init();
}

void printExternalSensorValues(int interval = 1000) {
  static unsigned long lastPrintTime = millis();
  if (millis() - lastPrintTime >= interval) {
    Serial.print("\n--- Sensors[Start] ---\n");
    Serial.printf("External pH sensor: %f\n", ph.ph());
    Serial.println("--- Sensors[End]   ---\n");
    lastPrintTime = millis();
  }
}

void loop() {
  cm.update();
  ph.update();
  printExternalSensorValues();
}
