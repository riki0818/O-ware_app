#include "Config.h"
#include "CoreModule.h"
#include "Utils.cpp"

CoreModule cm;
TDSSensor tds(cm, cm.A1);

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
  tds.init("/ex_tds");

  cm.begin();  // Start server
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

  printWiFiInfo();
  printExternalSensorValues();
  delay(1);
}
