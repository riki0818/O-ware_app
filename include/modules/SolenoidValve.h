#pragma once

#include "Base.h"

extern HardwareSerial Serial;

template <class ModuleType>
class SolenoidValve {
 private:
  int _pin;
  ModuleType& _module;
  bool _normalyOpen;
  bool _is_open;

 public:
  SolenoidValve(ModuleType& module, int pin, bool normalyOpen = false);
  void init(std::string path_prefix = "");
  void open();
  void close();
  bool is_open() { return _is_open; };
  void update();
};

template <class ModuleType>
SolenoidValve<ModuleType>::SolenoidValve(ModuleType& module, int pin, bool normalyOpen) : _module(module), _pin(pin), _normalyOpen(normalyOpen) {
  pinMode(_pin, OUTPUT);
  if (_normalyOpen) {
    _is_open = !_module.getPortState(_pin);
  } else {
    _is_open = _module.getPortState(_pin);
  }
}

template <class ModuleType>
void SolenoidValve<ModuleType>::open() {
  if (_normalyOpen) {
    _module.setPortState(_pin, false);
    _is_open = !_module.getPortState(_pin);
  } else {
    _module.setPortState(_pin, true);
    _is_open = _module.getPortState(_pin);
  }
}

template <class ModuleType>
void SolenoidValve<ModuleType>::close() {
  if (_normalyOpen) {
    _module.setPortState(_pin, true);
    _is_open = !_module.getPortState(_pin);
  } else {
    _module.setPortState(_pin, false);
    _is_open = _module.getPortState(_pin);
  }
}

template <class ModuleType>
void SolenoidValve<ModuleType>::init(std::string path_prefix) {
  if (path_prefix != "") {
    _module.addDigitalPortOutputEndpoint(
        path_prefix + std::string("/open"),
        _pin,
        _normalyOpen ? LOW : HIGH,
        [this]() { this->open(); },
        [this]() { return this->is_open(); });

    _module.addDigitalPortOutputEndpoint(
        path_prefix + std::string("/close"),
        _pin,
        _normalyOpen ? HIGH : LOW,
        [this]() { this->close(); },
        [this]() { return this->is_open(); });
  }
}

template <class ModuleType>
void SolenoidValve<ModuleType>::update() {
  if (_normalyOpen) {
    _is_open = !_module.getPortState(_pin);
  } else {
    _is_open = _module.getPortState(_pin);
  }
}