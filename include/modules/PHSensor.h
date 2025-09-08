#pragma once

#include "DFRobot_ESP_PH.h"

template <class ModuleType>
class PHSensor {
 private:
  int _channel;
  float _ph;
  ModuleType &_module;
  DFRobot_ESP_PH _phSensor;

 public:
  PHSensor(ModuleType &module, int channel)
      : _module(module), _channel(channel), _ph(7.0f) {}

  void init(std::string path = "") {
    _phSensor.begin();
    if (path != "") {
      _module.addGetValueEndpoint(
          [this]() { return this->ph(); },
          path,
          std::string("pH"));
    }
  }

  float ph() { return _ph; }

  void update(unsigned long interval = 1000) {
    static unsigned long lastUpdateTime = millis();
    if (millis() - lastUpdateTime >= interval) {
      float ESPADC = 4096.0f;
      float ESPVOLTAGE = 3300.0f;
      long voltage = _module.ADCread(_channel);
      voltage = voltage / ESPADC * ESPVOLTAGE;
      // readPH does not use temperature, so we use 25.0f as default
      // ref. https://github.com/GreenPonik/DFRobot_ESP_PH_BY_GREENPONIK/blob/731c09f1f8d724e1d400211fa811911c692f6735/src/DFRobot_ESP_PH.cpp#L60
      _ph = _phSensor.readPH(voltage, 25.0f);
      lastUpdateTime = millis();
    }
  }
};