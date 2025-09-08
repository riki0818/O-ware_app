#include "Config.h"
#include "CoreModule.h"
#include "Utils.cpp"

CoreModule cm;
Light light(cm, cm.D0_1);
Pump pump(cm, cm.D0_2);
SolenoidValve nc_sv(cm, cm.D1_1);
SolenoidValve no_sv(cm, cm.D1_2, true);  // If normally open, set the 3rd argment to true

void setup() {
  Serial.begin(115200);

  WiFi.begin(WiFiConf::ssid, WiFiConf::pass);
  int cnt = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    cnt += 1;
    if (cnt > 10) {
      ESP.restart();
    }
    Serial.print(".");
  }

  cm.init();

  light.init("/light");
  pump.init("/pump");
  nc_sv.init("/nc_sv");
  no_sv.init("/no_sv");

  cm.begin();
}

void printStates(int interval = 1000) {
  static unsigned long lastPrintTime = millis();
  if (millis() - lastPrintTime >= interval) {
    Serial.print("\n--- Port States[Start] ---\n");
    Serial.printf("Light: %s\n", light.is_on() ? "On" : "Off");
    Serial.printf("Pump: %s\n", pump.is_on() ? "On" : "Off");
    Serial.printf("NC Solenoid Valve: %s\n", nc_sv.is_open() ? "Open" : "Close");
    Serial.printf("NO Solenoid Valve: %s\n", no_sv.is_open() ? "Open" : "Close");
    Serial.println("--- Port States[End]   ---\n");
    lastPrintTime = millis();
  }
}

void loop() {
  cm.update();
  light.update();
  pump.update();
  nc_sv.update();
  no_sv.update();

  printStates();
  printWiFiInfo();
  delay(1);
}