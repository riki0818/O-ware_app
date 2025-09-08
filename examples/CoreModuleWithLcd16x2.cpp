#include "CoreModule.h"

CoreModule cm;
Lcd16X2 lcd(cm);

void setup() {
  Serial.begin(115200);
  while (!Serial);
  cm.init();
  lcd.init();

  // Line number starts from 0
  lcd.newLine(0, "Hello, World!");
}

void loop() {
  static int count = 0;
  static bool alignRightRow1 = false;
  static bool alignRightRow2 = false;
  cm.update();

  // isConnected() returns bool
  Serial.printf("LCD is connected: %s\n", lcd.isConnected() ? "true" : "false");

  // newLine(line, text) sets line and text
  lcd.newLine(1, "Count: ");
  // append(text) appends text at the end of the line which has set at last
  lcd.append(count, alignRightRow2);
  count++;

  if (count > 10) {
    count = 0;
    lcd.clear();

    alignRightRow1 = !alignRightRow1;
    alignRightRow2 = !alignRightRow2;

    // clear() clears all lines, so you need to set the line again
    lcd.newLine(0, "Hello, World!", alignRightRow1);
  }

  delay(1000);
}
