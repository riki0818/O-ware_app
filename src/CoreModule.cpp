#include "CoreModule.h"

#include <SPI.h>
#include <driver/pcnt.h>

CoreModule::CoreModule(Diameter diameter, int port)
    : Base(port) {
  _diameter = diameter;
}

void CoreModule::updateFlow()
/*
  Update flow [L/min]
*/
{
  static long m = millis();
  // Update flow every 1 second
  if (millis() - m > 1000) {
    _flowCountPerSec = getFlowCount();
    _flow = calculateFlow(_flowCountPerSec);
    m = millis();
  }
}

int16_t CoreModule::getFlowCount()
/*
  Get flow count.
*/
{
  int16_t flowCount = 0;
  pcnt_get_counter_value(PCNT_UNIT_0, &flowCount);

  if (flowCount > FLOW_COUNT_MAX) {
    flowCount = FLOW_COUNT_MAX;
  }

  pcnt_counter_pause(PCNT_UNIT_0);
  pcnt_counter_clear(PCNT_UNIT_0);
  pcnt_counter_resume(PCNT_UNIT_0);

  return flowCount;
}

float CoreModule::calculateFlow(float flow_count_per_sec)
/*
  Calculate flow [L/min] from flow count per second.
*/
{
  float flowLitterPerMin = 0;
  switch (_diameter) {
    case Diameter::Quarter:
      flowLitterPerMin = 0.02884 * _flowCountPerSec;
      break;
    case Diameter::ThreeEighth:
      flowLitterPerMin = 0.0683 * _flowCountPerSec - 0.3894;
      if (flowLitterPerMin < 0) flowLitterPerMin = 0;
      break;
    default:
      throw std::invalid_argument("Invalid diameter");
  }
  return flowLitterPerMin;
}

void CoreModule::updateTotalFlow() {
  static float prevFlow = 0;

  // _lastFlowUpdatedAt == 0 means just after constructed or reset.
  if (_lastFlowUpdatedAt == 0) {
    _lastFlowUpdatedAt = millis();
  }

  // Add an average between latest flow and previous flow to total flow multiplied by time
  float averageFlow = (_flow + prevFlow) / 2;
  unsigned long currentTime = millis();
  float timeDiff = static_cast<float>(currentTime - _lastFlowUpdatedAt) / 1000;

  // flow and prev_flow is L/min, time_diff is sec
  _totalFlow += (averageFlow + prevFlow) / 60 * timeDiff / 2;

  // Update last updated time and flow
  _lastFlowUpdatedAt = currentTime;
  prevFlow = _flow;
}

void CoreModule::resetTotalFlow()
/*
  Reset total flow.
*/
{
  _totalFlow = 0;
  _lastFlowUpdatedAt = 0;
}

void CoreModule::setTDSResistance(int i)
/*
  Set TDS resistance.
*/
{
  // for MAX4618
  if (i & 0x01)
    digitalWrite(Pin::TDS_RSEL0, 1);
  else
    digitalWrite(Pin::TDS_RSEL0, 0);
  if (i & 0x02)
    digitalWrite(Pin::TDS_RSEL1, 1);
  else
    digitalWrite(Pin::TDS_RSEL1, 0);
}

int CoreModule::calculateTDS(float voltage, int resistanceNo)
/*
  Calculate TDS [ppm] from voltage and resistance number.
*/
{
  float tds;
  if (_diameter == Diameter::Quarter) {
    switch (resistanceNo) {
      case 0:
	tds = 0.007909 * voltage * voltage + 1.4141 * voltage + 357.9580;
	break;
      case 1:
	tds = 0.5192 * voltage - 18.5847;
	break;
      case 2:
	tds = 0.04826 * voltage - 1.2852;
	break;
      case 3:
	tds = 0.005465 * voltage - 0.3315;
	break;
      default:
	Serial.println("calculateTDS with 1/4 diameter failed.");
	tds = 0.0;
    }
  } else if (_diameter == Diameter::ThreeEighth) {
    switch (resistanceNo) {
      case 0:
	tds = 0.006073 * voltage * voltage - 0.8759 * voltage + 420.1264;
	break;
      case 1:
	tds = 0.3020 * voltage - 4.5710;
	break;
      case 2:
	tds = 0.02695 * voltage - 1.5870;
	break;
      case 3:
	tds = 0.002669 * voltage - 0.9683;
	break;
      default:
	Serial.println("calculateTDS with 3/8 diameter failed.");
	tds = 0.0;
    }
  } else {
    throw std::invalid_argument("Invalid diameter");
  }

  return tds;
}

void CoreModule::updateTDS(int samples) {
  /*
    If an observed values is within the range (100 < value < 1500), update _tds.
    Otherwise, switch the resistance and wait for TDS_SETTLING_TIME milliseconds.
  */
  static int resistanceNo = 3;
  static unsigned long _lastSwitchedAt = 0;

  // Initialize resistance setting
  if (_lastSwitchedAt == 0)
    setTDSResistance(resistanceNo);

  // If TDS_SETTING_TIME milliseconds passes after switching at last, exit
  if (millis() - _lastSwitchedAt < TDS_SETTLING_TIME)
    return;

  // Get votages n_sample times and take average
  float totalVoltage = 0;
  for (int i = 0; i < samples; i++) {
    totalVoltage += ADCread(2);
  }
  float avgVoltage = totalVoltage / samples;

  // If observed value is within the scope, update _tds.
  // Otherwise, switch the resistance.
  if (avgVoltage >= 100 && avgVoltage <= 1500) {
    _tds = calculateTDS(avgVoltage, resistanceNo);
  } else if (avgVoltage > 1500 && resistanceNo != 0) {
    _lastSwitchedAt = millis();
    setTDSResistance(--resistanceNo);
  } else if (avgVoltage < 100 && resistanceNo != 3) {
    _lastSwitchedAt = millis();
    setTDSResistance(++resistanceNo);
  }
}

void CoreModule::updateTemperature()
/*
  Update temperature [℃].
*/
{
  uint16_t data;
  data = ADCread(0);
  if (data > 589.545) {
    _temperature = -0.03 * data + 67.66;
  } else {
    _temperature = -0.09 * data + 102.619;
  }
}

String CoreModule::getSensorValuesJson() {
  JsonDocument doc;
  doc["tds"] = _tds;
  doc["flow"] = _flow;
  doc["total_flow"] = _totalFlow;
  doc["temperature"] = _temperature;
  String json;
  serializeJson(doc, json);
  return json;
}

void CoreModule::update(int printInterval)
/*
  Update sensor values and print them if the diameter is not Null.
*/
{
  // Check timers
  this->checkTimer();

  if (_diameter != Diameter::Null) {
    static int printMillis = millis();

    // Update sensor values
    updateTemperature();
    updateFlow();
    updateTotalFlow();
    updateTDS();

    // Print sensor values
    if (millis() - printMillis > printInterval) {
      Serial.print("\n--- Preset Sensor Values[Start] ---\n");
      Serial.printf("Temperature: %.2f\n", _temperature);
      Serial.printf("Flow: %.2f\n", _flow);
      Serial.printf("Total Flow: %.2f\n", _totalFlow);
      Serial.printf("TDS: %d\n", _tds);
      Serial.println("--- Preset Sensor Values[End]   ---\n");
      printMillis = millis();
    }
  }
}

void CoreModule::addResetFlowEndpoint()
/*
  Add an endpoint to reset total flow.
*/
{
  this->on("/totalFlow/reset", HTTP_POST, [this](AsyncWebServerRequest *request) {
    try {
      this->resetTotalFlow();
        request->send(200, "application/json", this->createOperationSucceededResponse());
    }
    catch (std::exception &e) {
      request->send(500, "application/json", this->createErrorResponse(e.what()));
    } });
}

void CoreModule::init() {
  Base::init();

  pinMode(Pin::TDS_RSEL0, OUTPUT);
  digitalWrite(Pin::TDS_RSEL0, 0);

  pinMode(Pin::TDS_RSEL1, OUTPUT);
  digitalWrite(Pin::TDS_RSEL1, 0);

  // Clock for TDS drive
  ledcSetup(LEDC_CHANNEL_0, LEDC_BASE_FREQ, LEDC_TIMER_BIT);
  ledcAttachPin(Pin::TDS_CLK_PIN, LEDC_CHANNEL_0);
  ledcWrite(LEDC_CHANNEL_0, 0x80);

  // Counter for flow
  pcnt_config_t pcnt_config;
  pcnt_config.pulse_gpio_num = MH_FLOW;
  pcnt_config.ctrl_gpio_num = PCNT_PIN_NOT_USED;
  pcnt_config.lctrl_mode = PCNT_MODE_KEEP;
  pcnt_config.hctrl_mode = PCNT_MODE_KEEP;
  pcnt_config.channel = PCNT_CHANNEL_0;
  pcnt_config.unit = PCNT_UNIT_0;
  pcnt_config.pos_mode = PCNT_COUNT_INC;
  pcnt_config.neg_mode = PCNT_COUNT_DIS;
  pcnt_config.counter_h_lim = 1000;
  pcnt_config.counter_l_lim = -1000;

  pcnt_unit_config(&pcnt_config);
  pcnt_counter_pause(PCNT_UNIT_0);
  pcnt_counter_clear(PCNT_UNIT_0);
  pcnt_counter_resume(PCNT_UNIT_0);

  // Initialize digital port states
  states[Pin::D0_1] = false;
  states[Pin::D0_2] = false;
  states[Pin::D1_1] = false;
  states[Pin::D1_2] = false;

  pinMode(Pin::LED, OUTPUT);

  // Add an endpoint to get a sensor value
  addGetValueEndpoint([this]() { return this->getTDS(); },
                      "/tds", "ppm");
  addGetValueEndpoint([this]() { return this->getFlow(); },
                      "/flow", "L/min");
  addGetValueEndpoint([this]() { return this->getTotalFlow(); },
                      "/totalFlow", "L");
  addGetValueEndpoint([this]() { return this->getTemperature(); },
                      "/temperature", "celcius");
  addResetFlowEndpoint();

  this->onNotFound([this](AsyncWebServerRequest *request) { this->notFound(request); });
}

void CoreModule::checkTimer()
/*
  Check Timer object and if Timer.time is expired, set pin to Timer.mode.
*/
{
  for (auto const &[pinNumber, timer] : this->timers) {
    if (millis() >= timer.time) {
      digitalWrite(pinNumber, timer.mode);

      switch (pinNumber) {
	case Pin::D0_1:
	  setD0_1(timer.mode == HIGH);
	  break;
	case Pin::D0_2:
	  setD0_2(timer.mode == HIGH);
	  break;
	case Pin::D1_1:
	  setD1_1(timer.mode == HIGH);
	  break;
	case Pin::D1_2:
	  setD1_2(timer.mode == HIGH);
	  break;
      }
    }
  }
}