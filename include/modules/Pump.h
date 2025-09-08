#pragma once

#include "Base.h"

extern HardwareSerial Serial;

template <class ModuleType>
class Pump {
 private:
  int _pin;
  ModuleType& _module;
  bool _is_on;

 public:
  Pump(ModuleType& module, int pin);
  void init(std::string path_prefix = "");
  void on();
  void off();
  bool is_on() { return _is_on; };
  void update();
};

template <class ModuleType>
Pump<ModuleType>::Pump(ModuleType& module, int pin) : _module(module), _pin(pin) {
  pinMode(_pin, OUTPUT);
  _is_on = _module.getPortState(_pin);
}

template <class ModuleType>
void Pump<ModuleType>::on() {
  _module.setPortState(_pin, true);
  _is_on = _module.getPortState(_pin);
}

template <class ModuleType>
void Pump<ModuleType>::off() {
  _module.setPortState(_pin, false);
  _is_on = _module.getPortState(_pin);
}

template <class ModuleType>
void Pump<ModuleType>::init(std::string path_prefix) {
  if (path_prefix != "") {
    _module.addDigitalPortOutputEndpoint(
        path_prefix + std::string("/on"),
        _pin,
        HIGH,
        [this]() { this->on(); },
        [this]() { return this->is_on(); });

    _module.addDigitalPortOutputEndpoint(
        path_prefix + std::string("/off"),
        _pin,
        LOW,
        [this]() { this->off(); },
        [this]() { return this->is_on(); });
  }
}

template <class ModuleType>
void Pump<ModuleType>::update() {
  _is_on = _module.getPortState(_pin);
}