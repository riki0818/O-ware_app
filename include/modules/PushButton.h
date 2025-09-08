#pragma once

template <class ModuleType>
class PushButton {
 private:
  int _buttonPin;
  int _ledPin;
  ModuleType& _module;

  bool _isPressed = false;
  bool _isOn = false;

 public:
  PushButton(ModuleType& module, typename ModuleType::PortBase& port);
  void update();
  bool isOn() { return _isOn; };
  void on() {
    _isOn = true;
    digitalWrite(_ledPin, HIGH);
  };
  void off() {
    _isOn = false;
    digitalWrite(_ledPin, LOW);
  };
};

template <class ModuleType>
PushButton<ModuleType>::PushButton(ModuleType& module, typename ModuleType::PortBase& port) : _module(module), _buttonPin(port.port2()), _ledPin(port.port1()) {
  // Initialize the button and LED pins
  pinMode(_buttonPin, INPUT);
  pinMode(_ledPin, OUTPUT);

  // Turn off the LED
  digitalWrite(_ledPin, LOW);
}

template <class ModuleType>
void PushButton<ModuleType>::update() {
  // If the button is released, toggle the state
  if (digitalRead(_buttonPin) == LOW) {
    _isPressed = true;
  } else if (_isPressed) /* && digitalRead(_buttonPin) == HIGH */ {
    _isOn = !_isOn;
    _isPressed = false;

    if (_isOn) {
      digitalWrite(_ledPin, HIGH);
    } else {
      digitalWrite(_ledPin, LOW);
    }
  }
}
