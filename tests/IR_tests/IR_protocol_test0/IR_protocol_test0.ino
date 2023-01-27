#include <TowerRobot.h>

/* Initialize IRT object */
TowerRobot::IRT irt = TowerRobot::IRT(0, 3, 5);

void setup(void) {
  Serial.begin(9600);

  irt.begin();
}

void loop(void) {
  irt.send(1, IR_POLL_STATUS, 0xFF);
  delay(5000);
}