#include <PubSubClient.h>

#include "Config.h"
#include "CoreModule.h"
#include "Utils.cpp"

CoreModule cm(Diameter::Quarter);

WiFiClient wifiClient;
PubSubClient pubsubClient(wifiClient);

namespace MQTTConf {
// An example for Thingsboard
// -- Fixed parameters --
const char* server = "mqtt.thingsboard.cloud";
const int port = 1883;
const char* topic = "v1/devices/me/telemetry";
const char* client_id = "client-id";
const char* password = NULL;
// -- Fixed parameters --
}  // namespace MQTTConf

void setup() {
  Serial.begin(115200);

  // Connect to WiFi
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
  cm.begin();

  // Set MQTT broker property
  pubsubClient.setServer(MQTTConf::server, MQTTConf::port);
}

void publish() {
  static unsigned long lastPublishTime = millis();
  if (millis() - lastPublishTime > cm.publishInterval()) {
    // Create a payload
    JsonDocument payload = cm.metadata();
    payload["flow"] = cm.getFlow();
    payload["temperature"] = cm.getTemperature();
    payload["tds"] = cm.getTDS();

    // Output logs
    serializeJson(payload, Serial);
    Serial.println();

    // Publish to MQTT broker
    char buffer[1024];
    serializeJson(payload, buffer);
    pubsubClient.publish(MQTTConf::topic, buffer);
    lastPublishTime = millis();
  }
}

void loop() {
  cm.update();

  // Connect to mqtt broker if not connected
  if (cm.isPublishing()) {
    if (!pubsubClient.connected()) {
      pubsubClient.connect(MQTTConf::client_id, cm.clientId().c_str(), NULL);
    }
    publish();
  } else {
    if (pubsubClient.connected()) {
      pubsubClient.disconnect();
    }
  }

  printWiFiInfo();
  delay(1);
}