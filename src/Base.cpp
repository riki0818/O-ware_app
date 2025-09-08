#include <Base.h>

#include <sstream>
#include <unordered_map>

extern HardwareSerial Serial;

Base::Base(int port)
    : AsyncWebServer(port) {
}

void Base::init() {
  // Initialize SPI on ESP32 Arduino. It is used to read raw ADC data.
  initializeADC();

  // Add a header to allow cross-origin requests
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");

  // Add an endpoint to get local IP address
  addGetValueEndpoint([this]() { return WiFi.localIP(); },
                      "/config/ip");

  // Add endpoints for MQTT broker
  addPublishStartEndpoint();
  addPublishEndEndpoint();
}

void Base::initializeADC() {
  pinMode(getADC_CSb(), OUTPUT);
  digitalWrite(getADC_CSb(), HIGH);

  SPI.begin(
      getADC_SCK(),
      getADC_MISO(),
      getADC_MOSI(),
      getADC_CSb());

  SPI.beginTransaction(SPISettings(100000, MSBFIRST, SPI_MODE0));
}

void Base::addPublishStartEndpoint() {
  /*
      Add an endpoint to start to publish data to MQTT broker.
      Parameters in query strings except followings are used as a payload to publish
      - interval: int (optional) - interval to publish data in milliseconds
      - client_id: string (optional) - client ID to publish data
  */

  // Add endpoint
  std::string path = "/publish/start";
  this->on(path.c_str(), HTTP_POST, [this, path](AsyncWebServerRequest* request) {
        int time = millis();
        this->_clientId = ""; // initialize

        String response;
        int statusCode;
        try {
            // Construct a payload to publish
            int paramsNum = request->params();
            for (int i = 0; i < paramsNum; i++) {
                AsyncWebParameter* p = request->getParam(i);

                if (p->name() == "interval") {
                    this->_publishInterval = p->value().toInt();
                } else if (p->name() == "client_id") {
                    this->_clientId = std::string(p->value().c_str());
                } else {
                    this->_metadata[p->name()] = p->value();
                }
            }

            // Return an error if client_id is not set
            if (this->_clientId.empty()) {
                throw std::invalid_argument("client_id is required");
            }

            this->_isPublishing = true;

            response = this -> createOperationSucceededResponse();

            statusCode = 200;
            this -> printLog(statusCode, request->url().c_str(), response);
            request->send(statusCode, "application/json", response);
        } catch (std::exception &e) {
            statusCode = 500;
            response = createErrorResponse(e.what());
            this -> printLog(statusCode, request->url().c_str(), response);
            request->send(statusCode, "application/json", response);
        } });
}

void Base::addPublishEndEndpoint() {
  /*
      Add an endpoint to end publishing data to MQTT broker.
  */
  std::string path = "/publish/end";
  this->on(path.c_str(), HTTP_POST, [this, path](AsyncWebServerRequest* request) {
        int time = millis();

        // Construct a payload to publish
        this->_metadata.clear();
        this->_isPublishing = false;

        String response;
        int statusCode;
        try {
            response = this -> createOperationSucceededResponse();

            statusCode = 200;
            this -> printLog(statusCode, request->url().c_str(), response);
            request->send(statusCode, "application/json", response);
        } catch (std::exception &e) {
            statusCode = 500;
            response = createErrorResponse(e.what());
            this -> printLog(statusCode, request->url().c_str(), response);
            request->send(statusCode, "application/json", response);
        } });
}

void Base::notFound(AsyncWebServerRequest* request) {
  /*
      Add a handler for 404 error.
  */
  int time = millis();
  int statusCode = 404;
  std::string path = request->url().c_str();
  String response = createErrorResponse("Not found");
  this->printLog(statusCode, path, response);
  request->send(statusCode, "application/json", response);
}

std::unordered_map<std::string, std::string> Base::parseQueryString(const std::string& queryString) {
  /*
      Parse a query string to a map.
  */

  std::unordered_map<std::string, std::string> params;

  std::istringstream iss(queryString);
  std::string pair;

  while (std::getline(iss, pair, '&')) {
    std::istringstream pairStream(pair);
    std::string key, value;

    if (std::getline(std::getline(pairStream, key, '='), value)) {
      // URL decoding is required if the value contains special characters.
      params[key] = value;
    }
  }

  return params;
}

void Base::printLog(int statusCode, std::string path, String response, std::map<std::string, std::string> params) {
  /*
      Print a request log.
  */
  Serial.printf("Status Code: %d, ", statusCode);
  Serial.printf("Path: %s, ", path.c_str());
  Serial.printf("Response: %s, ", response.c_str());
  Serial.printf("Params: %s\n", params);
}

String Base::createOperationSucceededResponse() {
  /*
      Create a response for a successful operation.
  */

  JsonDocument doc;
  doc["result"] = "success";

  String serialized;
  serializeJson(doc, serialized);
  return serialized;
}

String Base::createErrorResponse(std::string detail) {
  /*
      Create a response for an error.
  */
  JsonDocument doc;
  doc["result"] = "error";
  doc["detail"] = detail;

  String serialized;
  serializeJson(doc, serialized);
  return serialized;
}

uint16_t Base::ADCread(uint8_t ch) {
  /*
      Read raw ADC data.
  */
  uint16_t data = 0;
  uint8_t dh, dl;
  digitalWrite(getADC_CSb(), LOW);
  SPI.transfer(0x06);
  dh = SPI.transfer(ch << 6);
  dl = SPI.transfer(0x00);
  digitalWrite(getADC_CSb(), HIGH);
  data = ((dh & 0x0f) << 8) | dl;

  return data;
}

void Base::addDigitalPortOutputEndpoint(
    std::string path,
    int pinNumber,
    int mode,
    std::function<void()> setter,
    std::function<boolean(void)> getter) {
  /*
      Add an endpoint to set a digital port state.
  */

  Serial.printf("Add an endpoint: %s\n", path.c_str());

  // Add endpoint
  this->on(path.c_str(), HTTP_POST,
           [path, pinNumber, setter, getter, mode, this](AsyncWebServerRequest* request) {
        int time = millis();

        unsigned long duration;
        const char param[] = "duration";
        bool invalid = false;
        if (request->hasParam(param)) {
            // String.toInt() returns 0 if the string is not a valid number.
            duration = request->getParam(param)->value().toInt();
        } else {
            duration = INT_MAX;
        }

        // Validate
        if (duration < 0) {
            int statusCode = 400;
            std::string detail = "duration is required to be int >= 0";
            String response = createErrorResponse(detail);
            this -> printLog(statusCode, request->url().c_str(), response);

            request -> send(statusCode, "application/json", response);
            return;
        }

        String response;
        int statusCode;
        int currentState = getter();
        try {
            if (duration > 0) {
                setter();
            }
            response = this -> createOperationSucceededResponse();

            // Remove timer if exists.
            if (this -> timers.count(pinNumber) > 0) {
                this -> timers.erase(pinNumber);
            }

            // Set timer if duration is set and the state before updating is different from the target state.
            if (duration > 0 && duration < INT_MAX) {
                int targetMode = mode == HIGH ? LOW : HIGH;
                this -> timers.emplace(pinNumber, Timer{millis() + duration, targetMode});
            }

            // Writing Serial.print does not work, so use a function to print logs instead.
            statusCode = 200;
            this -> printLog(statusCode, request->url().c_str(), response);
            request->send(statusCode, "application/json", response);
        } catch (std::exception &e) {
            statusCode = 500;
            response = createErrorResponse(e.what());
            this -> printLog(statusCode, request->url().c_str(), response);
            request->send(statusCode, "application/json", response);
        } });
}

void Base::addOperationEndpoint(std::function<void(void)> fn, std::string path) {
  /*
      Add an endpoint to execute an operation.
  */
  this->on(path.c_str(), HTTP_POST, [path, fn, this](AsyncWebServerRequest* request) {
        String response;
        int statusCode;
        try {
            fn();
            statusCode = 200;
            response = this -> createOperationSucceededResponse();
            this -> printLog(statusCode, request->url().c_str(), response);
            request->send(statusCode, "application/json", response);
        } catch (std::exception &e) {
            statusCode = 500;
            response = createErrorResponse(e.what());
            this -> printLog(statusCode, request->url().c_str(), response);
            request->send(statusCode, "application/json", response);
        } });
}

void Base::addOperationEndpoint(std::function<void(float)> fn, std::string path, std::string param) {
  /*
      Add an endpoint to execute an operation with a float parameter.
  */
  this->on(path.c_str(), HTTP_POST, [path, fn, param, this](AsyncWebServerRequest* request) {
        String response;
        int statusCode;
        try {
            float *value = nullptr;
            if (request->hasParam(param.c_str())) {
                value = new float();
                *value = request->getParam(param.c_str())->value().toFloat();
            }

            fn(*value);
            response = this -> createOperationSucceededResponse();

            statusCode = 200;
            this -> printLog(statusCode, request->url().c_str(), response);
            request->send(statusCode, "application/json", response);
        } catch (std::exception &e) {
            statusCode = 500;
            response = createErrorResponse(e.what());
            this -> printLog(statusCode, request->url().c_str(), response);
            request->send(statusCode, "application/json", response);
        } });
}