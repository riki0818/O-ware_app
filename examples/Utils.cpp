#include <Arduino.h>
#include <WiFi.h>

void printWiFiInfo(int interval = 1000) {
  static int lastPrintTime = millis();
  if (millis() - lastPrintTime > interval) {
    Serial.print("\n--- WiFi Information[Start] ---\n");
    Serial.print("WiFi status: ");
    Serial.println(WiFi.status());
    Serial.print("WiFi localIP: ");
    Serial.println(WiFi.localIP());
    Serial.print("MAC Address:  ");
    Serial.println(WiFi.macAddress());
    Serial.println("--- WiFi Information[End]   ---\n");
    lastPrintTime = millis();
  }
}

void reconnectWifiIfDisconnected(int interval = 30000)
/* `interval` should be larger than time to take until connected. */
{
  static unsigned long lastReconnectTime = millis();
  if (WiFi.status() != WL_CONNECTED && millis() - lastReconnectTime > interval) {
    Serial.println("WiFi disconnected. Reconnecting...");
    WiFi.reconnect();
    lastReconnectTime = millis();
  }
}