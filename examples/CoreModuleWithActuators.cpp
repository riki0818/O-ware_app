#include "CoreModule.h"

CoreModule cm;
Light light(cm, cm.D0_1);
Pump pump(cm, cm.D0_2);
SolenoidValve nc_sv(cm, cm.D1_1);
SolenoidValve no_sv(cm, cm.D1_2, true);  // If normally open, set the 3rd argment to true

void setup() {
  Serial.begin(115200);
  cm.init();
}

void printStates() {
  Serial.print("\n--- Port States[Start] ---\n");
  Serial.printf("Light: %s\n", light.is_on() ? "On" : "Off");
  Serial.printf("Pump: %s\n", pump.is_on() ? "On" : "Off");
  Serial.printf("NC Solenoid Valve: %s\n", nc_sv.is_open() ? "Open" : "Close");
  Serial.printf("NO Solenoid Valve: %s\n", no_sv.is_open() ? "Open" : "Close");
  Serial.println("--- Port States[End]   ---\n");
}

void loop() {
  cm.update();

  light.on();
  delay(1000);
  printStates();

  pump.on();
  delay(1000);
  printStates();

  nc_sv.open();
  delay(1000);
  printStates();

  no_sv.close();
  delay(1000);
  printStates();

  light.off();
  delay(1000);
  printStates();

  pump.off();
  delay(1000);
  printStates();

  nc_sv.close();
  delay(1000);
  printStates();

  no_sv.open();
  delay(1000);
  printStates();
}