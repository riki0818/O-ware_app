#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>
#include <PubSubClient.h>
#include <SPI.h>
#include <WiFi.h>

#include <map>
#include <string>

#include "modules/Lcd16x2.h"
#include "modules/Light.h"
#include "modules/PHSensor.h"
#include "modules/PressureSensor.h"
#include "modules/Pump.h"
#include "modules/PushButton.h"
#include "modules/SolenoidValve.h"
#include "modules/TDSSensor.h"

struct Timer {
  unsigned long time;
  int mode;
};

class Base : public AsyncWebServer {
 private:
  int _port;

  std::unordered_map<std::string, std::string> parseQueryString(const std::string& queryString);
  void addPublishStartEndpoint();
  void addPublishEndEndpoint();

  // MQTT settings
  boolean _isPublishing = false;
  int _publishInterval = 100;
  std::string _clientId = "";
  JsonDocument _metadata;

 public:
  Base(int port);
  void init();

  boolean isPublishing() { return _isPublishing; }
  int publishInterval() { return _publishInterval; }
  std::string clientId() { return _clientId; }
  JsonDocument metadata() { return _metadata; }

  // [start] Methods for HTTP server
  template <typename Lambda>
  void addGetValueEndpoint(Lambda fn, std::string path, std::string unit = "");
  void addOperationEndpoint(std::function<void(void)> fn, std::string path);
  void addOperationEndpoint(std::function<void(float)> fn, std::string path, std::string param);

  void addDigitalPortOutputEndpoint(
      std::string path,
      int pinNumber,
      int mode,
      std::function<void()> setter,
      std::function<boolean(void)> getter);
  // [end] Methods for HTTP server

  uint16_t ADCread(uint8_t ch);

  boolean getPortState(int pinNumber) { return states[pinNumber]; }
  void setPortState(int pinNumber, boolean state) {
    digitalWrite(pinNumber, state);
    states[pinNumber] = state;
  }

  String createOperationSucceededResponse();
  String createErrorResponse(std::string detail);

  struct PortBase {
    virtual int port1() const = 0;
    virtual int port2() const = 0;
    virtual ~PortBase() = default;
  };

 protected:
  // Pin states
  std::map<int, boolean> states;

  void setPinState(int pinNumber, boolean state) {
    digitalWrite(pinNumber, state);
    states[pinNumber] = state;
  }

  // [start] Methods for HTTP server
  void notFound(AsyncWebServerRequest* request);

  template <typename T>
  String createSingleValueSucceededResponse(T value, std::string unit = "");
  void printLog(int statusCode, std::string path, String response, std::map<std::string, std::string> params = {});
  // [end] Methods for HTTP server

  std::map<int, Timer> timers;

  virtual int getADC_CSb() const = 0;
  virtual int getADC_SCK() const = 0;
  virtual int getADC_MISO() const = 0;
  virtual int getADC_MOSI() const = 0;

  void initializeADC();
};

template <typename Lambda>
void Base::addGetValueEndpoint(Lambda fn, std::string path, std::string unit) {
  this->on(path.c_str(), HTTP_GET, [path, fn, unit, this](AsyncWebServerRequest* request) {
                String response;
                int statusCode;
                try
                {
                    statusCode = 200;
                    response = this->createSingleValueSucceededResponse(fn(), unit);
                    this->printLog(statusCode, path, response);
                    request->send(statusCode, "application/json", response);
                }
                catch (std::exception &e)
                {
                    statusCode = 500;
                    response = this->createErrorResponse(e.what());
                    this->printLog(statusCode, path, response);
                    request->send(statusCode, "application/json", response);
                } });

  return;
};

template <typename T>
String Base::createSingleValueSucceededResponse(T value, std::string unit) {
  JsonDocument doc;
  doc["value"] = value;
  if (!unit.empty())
    doc["unit"] = unit;

  char serialized[1024];
  serializeJson(doc, serialized);
  return serialized;
}