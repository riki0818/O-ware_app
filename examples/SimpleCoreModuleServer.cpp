#include "Config.h"
#include "CoreModule.h"
#include "Utils.cpp"

CoreModule cm(Diameter::ThreeEighth);

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

  cm.begin();  // Begin the server
}

void loop() {
  cm.update();

  printWiFiInfo();
  delay(1);
}