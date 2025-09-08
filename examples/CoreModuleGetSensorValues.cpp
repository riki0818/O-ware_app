#include "CoreModule.h"

CoreModule cm(Diameter::Quarter);

void setup() {
  Serial.begin(115200);
  cm.init();
}

void loop() {
  static int count = 0;
  // Call in short intervals because it updates the sensor values
  // Argument is the interval to print the sensor values
  cm.update(20000);

  // Print sensor values every 1000 loops
  if (count > 1000) {
    count = 0;
    Serial.println("Sensor Values as Struct:");
    SensorValues sv = cm.getSensorValues();
    Serial.printf("TDS: %d\n", sv.tds);
    Serial.printf("Flow: %f\n", sv.flow);
    Serial.printf("Total Flow: %f\n", sv.totalFlow);
    Serial.printf("Temperature: %f\n", sv.temperature);
    Serial.println();

    Serial.println("Sensor Values as Json:");
    Serial.println(cm.getSensorValuesJson());
    Serial.println();
  }
  count++;

  delay(1);
}