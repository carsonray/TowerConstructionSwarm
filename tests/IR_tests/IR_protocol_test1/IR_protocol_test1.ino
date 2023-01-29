#include <TowerRobot.h>

/* Initialize IRT object */
TowerRobot::IRT irt = TowerRobot::IRT(1, 3, 5);

using namespace IRcommands;

void setup() {
  Serial.begin(9600);

  irt.begin();
}

void loop() {
  irt.send(2, IR_POLL_STATUS, 0xFF);
  delay(1000);
}