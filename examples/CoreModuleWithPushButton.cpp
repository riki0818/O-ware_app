#include "CoreModule.h"

CoreModule cm;
PushButton pb(cm, cm.D0);

void setup() {
  Serial.begin(115200);
  cm.init();
}

void printStates(int interval = 1000) {
  static unsigned long lastPrintTime = millis();
  if (millis() - lastPrintTime < interval) {
    return;
  }
  lastPrintTime = millis();
  Serial.print("\n--- Port States[Start] ---\n");
  Serial.printf("Push Button: %s\n", pb.isOn() ? "On" : "Off");
  Serial.println("--- Port States[End]   ---\n");
}

void loop() {
  cm.update();
  pb.update();
  printStates();
  delay(1);
}
