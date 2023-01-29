#include <TowerRobot.h>

/* Initialize IRT object */
TowerRobot::IRT irt = TowerRobot::IRT(1, 13, 5);

using namespace IRcommands;

void setup() {
  Serial.begin(9600);

  irt.begin();

  irt.send(0x2, 0x1, 0xFF);

  irt.setSendInterval(500, 5000);
  irt.setSendRepeats(-1);
}

void loop() {
  irt.update();
}