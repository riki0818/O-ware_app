#include "CoreModule.h"

CoreModule cm;
TDSSensor tds(cm, cm.A1);

void setup() {
  Serial.begin(115200);
  cm.init();
}

void printExternalSensorValues(int interval = 1000) {
  static unsigned long lastPrintTime = millis();
  if (millis() - lastPrintTime >= interval) {
    Serial.print("\n--- Sensors[Start] ---\n");
    Serial.printf("External TDS sensor: %f\n", tds.tds());
    Serial.println("--- Sensors[End]   ---\n");
    lastPrintTime = millis();
  }
}

void loop() {
  cm.update();
  tds.update();
  printExternalSensorValues();
}
