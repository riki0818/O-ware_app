#pragma once

template <class ModuleType>
class PressureSensor {
 private:
  int _channel;
  float _pressure;
  ModuleType &_module;

 public:
  PressureSensor(ModuleType &module, int channel);
  void init(std::string path = "");
  float pressure() { return _pressure; };
  void update(int n_sample = 10);
};

template <class ModuleType>
PressureSensor<ModuleType>::PressureSensor(ModuleType &module, int channel) : _module(module), _channel(channel) {}

template <class ModuleType>
void PressureSensor<ModuleType>::init(std::string path) {
  if (path != "") {
    _module.addGetValueEndpoint(
        [this]() { return this->pressure(); },
        path,
        std::string("psi"));
  }
}

template <class ModuleType>
void PressureSensor<ModuleType>::update(int n_sample) {
  /*
      Fetch pressure values at multiple times from the sensor
      and store the average into _pressure
  */
  float pressure_value_sum = 0;  // Unit: MPa
  for (int i = 0; i < n_sample; i++) {
    long voltage = _module.ADCread(_channel);
    pressure_value_sum += 0.000421 * voltage - 0.314433;
  }

  // Convert MPa to psi
  _pressure = pressure_value_sum / n_sample * 145;
}