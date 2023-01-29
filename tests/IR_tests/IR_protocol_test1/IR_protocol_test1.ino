#include <TowerRobot.h>

/* Initialize IRT object */
TowerRobot::IRT irt = TowerRobot::IRT(1, 13, 5);

using namespace IRcommands;

void setup() {
  Serial.begin(9600);

  irt.begin();
}

void loop() {
  irt.send(0, 0, 0x9);
  irt.update();
  delay(1000);
}