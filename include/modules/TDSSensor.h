#pragma once

#include <cstdint>
#include <string>

template <class ModuleType>
class TDSSensor {
 private:
  int _channel;
  int _tds = 0;
  ModuleType &_module;

 public:
  TDSSensor(ModuleType &module, int channel);
  void init(std::string path = "");
  float tds() { return _tds; };
  void update();
};

template <class ModuleType>
TDSSensor<ModuleType>::TDSSensor(ModuleType &module, int channel) : _module(module), _channel(channel) {}

template <class ModuleType>
void TDSSensor<ModuleType>::init(std::string path) {
  if (path != "") {
    _module.addGetValueEndpoint(
        [this]() { return this->tds(); },
        path,
        std::string("ppm"));
  }
}

template <class ModuleType>
void TDSSensor<ModuleType>::update() {
  long voltage = _module.ADCread(_channel);
  _tds = 0.4407 * voltage;
}