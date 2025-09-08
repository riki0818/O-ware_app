#include "CoreModule.h"

CoreModule cm(Diameter::Quarter);

void setup() {
  Serial.begin(115200);
  cm.init();
}

void loop() {
  cm.update();
  delay(1);
}