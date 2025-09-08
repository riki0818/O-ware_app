#pragma once

#include <list>
#include <string>

#include "Base.h"

enum class Diameter {
  Null,
  Quarter,
  ThreeEighth
};

struct SensorValues {
  int tds;
  float flow;
  float totalFlow;
  float temperature;
};

class CoreModule : public Base {
 private:
  // sensor values
  int _tds = 0;
  float _flow = 0;
  float _totalFlow = 0;
  float _temperature = 0;

 public:
  enum Pin {
    MH_FLOW = 4,
    ADC_CSb = 5,
    TDS_CLK_PIN = 12,
    TDS_RSEL0 = 26,
    TDS_RSEL1 = 27,
    D0_1 = 32,
    D0_2 = 33,
    D1_1 = 16,
    D1_2 = 17,
    ADC_SCK = 18,
    ADC_MISO = 19,
    I2C_SDA = 21,
    I2C_SCL = 22,
    ADC_MOSI = 23,
    LED = 25,
    RX_PIN = 13,
    TX_PIN = 14
  };

  enum AnalogPort {
    A0 = 3,
    A1 = 1,
  };

  CoreModule(Diameter diameter = Diameter::Null, int port = 80);

  void init();
  void update(int printInterval = 1000);

  // TDS
  void updateTDS(int samples = 5);
  int getTDS() { return _tds; };

  // Flow
  void updateFlow();
  void updateTotalFlow();
  void resetTotalFlow();
  float getFlow() { return _flow; };
  float getTotalFlow() { return _totalFlow; };

  // Temperature
  void updateTemperature();
  float getTemperature() { return _temperature; };

  // All sensor values
  SensorValues getSensorValues() { return {_tds, _flow, _totalFlow, _temperature}; };
  String getSensorValuesJson();

  // Analog port reader
  uint16_t readA0() { return ADCread(AnalogPort::A0); };
  uint16_t readA1() { return ADCread(AnalogPort::A1); };

  // Digital port setter
  void setD0_1(boolean state) { setPinState(Pin::D0_1, state); };
  void setD0_2(boolean state) { setPinState(Pin::D0_2, state); };
  void setD1_1(boolean state) { setPinState(Pin::D1_1, state); };
  void setD1_2(boolean state) { setPinState(Pin::D1_2, state); };

  // Digital port getter
  boolean getD0_1() { return states[Pin::D0_1]; };
  boolean getD0_2() { return states[Pin::D0_2]; };
  boolean getD1_1() { return states[Pin::D1_1]; };
  boolean getD1_2() { return states[Pin::D1_2]; };

 private:
  Diameter _diameter;

  // PWM parameters for TDS
  const int LEDC_CHANNEL_0 = 0;
  const int LEDC_TIMER_BIT = 8;
  const float LEDC_BASE_FREQ = 2400.0;
  const int TDS_SETTLING_TIME = 150;

  const int FLOW_COUNT_MAX = 60000;

  void addResetFlowEndpoint();
  std::string pinToString(Pin value);
  void checkTimer();

  // TDS
  void setTDSResistance(int i);
  int calculateTDS(float voltage, int resistanceNo);

  // Flow
  int16_t getFlowCount();
  float calculateFlow(float flowCountPerSec);
  unsigned long _lastFlowUpdatedAt = 0;
  uint16_t _flowCountPerSec = 0;

  // Digital port
  struct _D0 : public PortBase {
    int port1() const override { return Pin::D0_1; }
    int port2() const override { return Pin::D0_2; }
  };

  struct _D1 : public PortBase {
    int port1() const override { return Pin::D1_1; }
    int port2() const override { return Pin::D1_2; }
  };

 protected:
  // Override ADC pin getter to initialize ADCread()
  int getADC_CSb() const override { return static_cast<int>(Pin::ADC_CSb); }
  int getADC_SCK() const override { return static_cast<int>(Pin::ADC_SCK); }
  int getADC_MISO() const override { return static_cast<int>(Pin::ADC_MISO); }
  int getADC_MOSI() const override { return static_cast<int>(Pin::ADC_MOSI); }

 public:
  // Digital port
  _D0 D0;
  _D1 D1;
};
