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

void togglePushButton() {
  static unsigned long lastToggleTime = 0;
  if (millis() - lastToggleTime >= 1000) {
    if (pb.isOn()) {
      pb.off();
    } else {
      pb.on();
    }
    lastToggleTime = millis();
  }
}

void loop() {
  cm.update();
  pb.update();
  togglePushButton();
  printStates();
  delay(1);
}
