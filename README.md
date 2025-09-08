# O-Library

An ESP32-based library providing modular components for IoT and automation systems. Supports for various sensors and actuators.


## Key Features

### CoreModule
- Built-in sensor support:
  - Temperature
  - Flow rate/volume
  - TDS (Total Dissolved Solids)
- 2 Digital I/O pairs
- 2 Analog inputs
- HTTP API endpoints for monitoring and control

### External Modules
This repository provides following external modules.

- Actuators
  - Light (LED) 
  - Pump
  - Solenoid Valve (Normally Closed and Normally Open)
  - [LCD (16x2)](https://wiki.seeedstudio.com/Grove-LCD_RGB_Backlight/)
- Sensors
  - TDS Sensor
    - [Grove TDS Sensor](https://wiki.seeedstudio.com/Grove-TDS-Sensor/)
  - Pressure Sensor
    - [XDB302](https://www.xidibei.com/en-jp/products/xdb302-pressure-transducer)
  - pH Sensor
    - [Gravity: Analog pH Sensor/Meter Kit V2](https://www.switch-science.com/products/5040)
- Others
  - Push Button

## Setup
1. If you haven't already, install [PlatformIO](https://platformio.org/).
2. Clone or download this repository to your local machine.
3. Open the project folder in PlatformIO.
4. Select your target example code and upload it to Core Module.

### Dependencies
- [zeed/ESP Async WebServer@^1.2.3](https://github.com/me-no-dev/ESPAsyncWebServer)
- [bblanchon/ArduinoJson@^7.1.0](https://github.com/bblanchon/ArduinoJson)
- [knolleary/PubSubClient@^2.8](https://github.com/knolleary/pubsubclient)
- [Seeed-Studio/Grove_LCD_RGB_Backlight@^1.0.2](https://github.com/Seeed-Studio/Grove_LCD_RGB_Backlight)

## Usage
This library is designed to simplify building applications for both the Core Module and external modules.

All modules come with a built-in HTTP server that can be accessed through the ESP32 board's IP address. You can interact with the server using a web browser or REST client to control components and monitor sensor data.

For detailed API documentation, please refer to `./docs/openapi.yaml`.

### CoreModule
#### Including the Library
First, include the CoreModule header file in your sketch:

```cpp
#include "CoreModule.h"
```

#### Module Initialization
Create a CoreModule instance. You can optionally specify the tube diameter and port number. Don't forget to call `init()` in your setup function.

```cpp
// Basic initialization
CoreModule cm;

// With tube diameter specified
CoreModule cm(Diameter::Quarter);  // 1/4 inch tube

// With both tube diameter and custom port
CoreModule cm(Diameter::ThreeEighth, 8080);  // 3/8 inch tube, API server on port 8080

void setup() {
  cm.init();
}
```

#### HTTP API Server Setup
To enable the HTTP API server, call `begin()` after initialization:

```cpp
void setup() {
  cm.init();
  cm.begin();
}
```

#### Reading Sensor Data
Call `update()` in your main loop to refresh sensor readings:

```cpp
void loop() {
  // Update readings every 1000ms and print to serial monitor
  cm.update(1000);
  
  // Access the latest sensor readings
  float temperature = cm.getTemperature();
  float flow = cm.getFlow();
  float totalFlow = cm.getTotalFlow();
  int tds = cm.getTDS();
}
```

#### Available Pins
CoreModule provides these pins for your use:
```cpp
cm.D0_1
cm.D0_2
cm.D1_1
cm.D1_2
cm.A0
cm.A1
```

#### Core Functions
##### Digital I/O Methods
- `boolean getD0_1()`, `getD0_2()`, `getD1_1()`, `getD1_2()`
  - Read the state of digital GPIO pins (returns `true` for HIGH, `false` for LOW)
- `void setD0_1(boolean)`, `setD0_2(boolean)`, `setD1_1(boolean)`, `setD1_2(boolean)`
  - Set the state of digital GPIO pins (`true` for HIGH, `false` for LOW)

##### Analog Input Methods
- `uint16_t readA0()`, `readA1()`
  - Read the value from analog input ports

##### Sensor Reading/Manipulating Methods
- `int getTDS()`
  - Get the current TDS (Total Dissolved Solids) reading in `ppm` unit.
- `float getFlow()`
  - Get the current flow rate in `L/min` unit.
- `float getTotalFlow()`
  - Get the accumulated total flow volume in `L` unit.
- `float getTemperature()`
  - Get the current temperature reading in `°C` unit.
- `void resetTotalFlow()`
  - Reset the accumulated flow volume to zero

### TDS Sensor

#### Getting Started
To use the TDS sensor, first create a TDSSensor instance by specifying your CoreModule and the analog pin where the sensor is connected:

```cpp
CoreModule cm;
TDSSensor tdsSensor(cm, cm.A1);
```

#### HTTP API Support (Optional)
To make sensor readings available via HTTP, initialize an endpoint by providing a path prefix:

```cpp
tdsSensor.init("/tds");
```

This creates a REST endpoint at `/tds` that returns the current TDS reading in parts per million (ppm).

#### Reading Sensor Data
In your main loop, regularly update and read the sensor:

```cpp
void loop() {
  tdsSensor.update();
  float tdsValue = tdsSensor.tds();

  // Do something with the reading
  Serial.printf("TDS: %.2f ppm\n", tdsValue);
}
```

Note: The `update()` method must be called regularly to get fresh readings from the sensor.

### Light Component
#### Initialization
To use a light module, create a Light object by specifying your module and the pin number:

```cpp
Light light(cm, cm.D0_1); // Use appropriate pin number
```

#### Basic Control
The light can be controlled using simple on/off methods. You can also check its current state:

```cpp
light.on();  // Turn on
delay(1000); // Wait 1 second
light.off(); // Turn off

// Check current state
if (light.is_on()) {
    // Light is currently on
} else {
    // Light is currently off
}
```

#### Core Methods
##### void on()
Activates the light by setting the pin to HIGH.

##### void off()
Deactivates the light by setting the pin to LOW.

##### bool is_on()
Returns the current state (true = on, false = off).

##### void update()
Updates the internal state by checking the current pin value. Please refer to the Timed Control section for more details.

#### HTTP API Integration
Enable HTTP control by initializing with an endpoint path:

```cpp
light.init("/light");
```

#### Timed Control
The HTTP API supports timed operations using a duration parameter (in milliseconds). For example, sending a request with `duration=5000` will toggle the light for 5 seconds.

**Note:** To use timed control, you must call `light.update()` in your main loop. Without this, the reported state may not match the actual light state.

### Pump
#### Initialization
Create a Pump instance by specifying your module and the GPIO pin for the pump:

```cpp
// CoreModule example
CoreModule cm;
Pump pump(cm, cm.D0_1); // Specify the desired GPIO pin
```

#### Basic Control
Control the pump using simple on/off methods and check its current state:

```cpp
pump.on();  // Start the pump
delay(1000); // Wait 1 second
pump.off(); // Stop the pump

// Check pump status
if (pump.is_on()) {
    // Pump is running
} else {
    // Pump is stopped
}
```

#### Core Methods
##### void on()
Starts the pump by setting the GPIO pin HIGH.

##### void off()
Stops the pump by setting the GPIO pin LOW.

##### bool is_on()
Returns the current state (true = running, false = stopped).

##### void update()
Updates the internal state by checking the current pin value. Required for timed control operations.

#### HTTP API Integration
Enable HTTP control by initializing with an endpoint path:

```cpp
pump.init("/pump");
```

#### Timed Control
The HTTP API supports timed operations using a duration parameter (in milliseconds). For example, sending a request with `duration=5000` will run the pump for 5 seconds.

**Note:** To use timed control, you must call `pump.update()` in your main loop. Without this, the reported state may not match the actual pump state.

### Solenoid Valve
#### Getting Started
To use a solenoid valve, create a `SolenoidValve` instance by specifying your module, pin number, and valve type (normally open or closed):

```cpp
CoreModule cm;
SolenoidValve valve(cm, cm.D0_1);  // Defaults to normally closed
SolenoidValve valve(cm, cm.D0_1, false);  // Explicitly normally closed
SolenoidValve valve(cm, cm.D0_1, true);   // Normally open


#### Basic Control
Control the valve using simple open/close methods and check its current state:

```cpp
valve.open();  // Opens the valve
delay(1000);   // Wait 1 second
valve.close(); // Closes the valve

// Check valve status
if (valve.is_open()) {
    // Valve is open
} else {
    // Valve is closed
}
```

#### Core Methods
##### void open()
Opens the valve by setting the appropriate pin state based on the valve type.

##### void close()
Closes the valve by setting the appropriate pin state based on the valve type.

##### bool is_open()
Returns the current state (true = open, false = closed).

##### void update()
Updates the internal state by checking the current pin value. Required for timed control operations.

#### HTTP API Integration
Enable HTTP control by initializing with an endpoint path:

```cpp
valve.init("/valve");
```

#### Timed Control
The HTTP API supports timed operations using a duration parameter (in milliseconds). For example, sending a request with `duration=5000` will open/close the valve for 5 seconds.

**Note:** To use timed control, you must call `valve.update()` in your main loop. Without this, the reported state may not match the actual valve state.

### Pressure Sensor (XDB302)
The XDB302 pressure transducer can be easily integrated into your project.

#### Basic Setup
Create a pressure sensor instance by specifying your module and analog input pin:

```cpp
CoreModule cm;
PressureSensor ps(cm, cm.A1);  // Connect to analog pin A1
```

#### Core Methods
##### float pressure()
Returns the current pressure reading in PSI.

##### void update()
Refreshes the pressure reading. Call this method regularly in your main loop.

#### HTTP API Support (Optional)
Enable HTTP endpoints by initializing with a path prefix:

```cpp
ps.init("/pressure");
```

### TDS Sensor (Grove)
The Grove TDS (Total Dissolved Solids) sensor measures water quality.

#### Basic Setup
Create a TDS sensor instance by specifying your module and analog input pin:

```cpp
CoreModule cm;
TDSSensor tds(cm, cm.A1);  // Connect to analog pin A1
```

#### Core Methods
##### float tds()
Returns the current TDS reading in PPM (parts per million).

##### void update()
Refreshes the TDS reading. Call this method regularly in your main loop.

#### HTTP API Support (Optional)
Enable HTTP endpoints by initializing with a path prefix:

```cpp
tds.init("/tds");
```

### pH Sensor (Gravity: Analog pH Sensor/Meter Kit V2)
The DFRobot Gravity pH sensor provides accurate pH measurements for your aqueous solutions.

#### Basic Setup
Create a pH sensor instance by specifying your module and analog input pin:

```cpp
CoreModule cm;
PHSensor ph(cm, cm.A1);  // Connect to analog pin A1
```

#### Core Methods
##### float pH()
Returns the current pH reading (0-14 scale).

##### void update(unsigned long interval = 1000)
Refreshes the pH reading. Call this method regularly in your main loop.

#### HTTP API Support (Optional)
Enable HTTP endpoints by initializing with a path prefix:

```cpp
ph.init("/ph");
```

### Push Button
The Push Button module can be used to toggle the state of a digital output port.

#### Basic Setup
Create a PushButton instance by specifying your module and the digital input pin:

```cpp
CoreModule cm;
PushButton pb(cm, cm.D0);  // Connect to digital pin D0
```

#### Core Methods
##### bool isOn()
Returns the current state (true = on, false = off).

##### void update()
Updates the internal state by checking the current pin value.

##### void on()
Turns on the push button.

##### void off()
Turns off the push button.

### LCD (16x2)
The Grove LCD module provides a 16x2 character display with RGB backlight.

#### Basic Setup
Create an LCD instance by specifying your module:

```cpp
CoreModule cm;
Lcd16X2 lcd(cm);
```

#### Core Methods
##### void init()
Initializes the LCD module. Call this method in your setup function.

##### void clear()
Clears all text from the LCD display.

##### void newLine(int line, const char* text)
Sets text on the specified line (0 or 1). The text will be truncated if longer than 16 characters.

##### void append(const char* text)
Appends text to the end of the last line that was set using newLine().

##### void append(int value)
Appends a number to the end of the last line that was set using newLine().

##### bool isConnected()
Returns whether the LCD is properly connected and responding (true = connected, false = disconnected).

## Contributing

Contributions are welcome! If you find any issues or have suggestions for improvements, please open an issue or submit a pull request.
