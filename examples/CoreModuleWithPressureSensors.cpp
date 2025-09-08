#include "CoreModule.h"

CoreModule cm;
PressureSensor pressure(cm, cm.A1);

void setup() {
  Serial.begin(115200);
  cm.init();
}

void printExternalSensorValues(int interval = 1000) {
  static unsigned long lastPrintTime = millis();
  if (millis() - lastPrintTime >= interval) {
    Serial.print("\n--- Sensors[Start] ---\n");
    Serial.printf("External Pressure sensor: %f\n", pressure.pressure());
    Serial.println("--- Sensors[End]   ---\n");
    lastPrintTime = millis();
  }
}

void loop() {
  cm.update();
  pressure.update();
  printExternalSensorValues();
}
