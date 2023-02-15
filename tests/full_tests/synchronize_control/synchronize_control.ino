#include <TowerRobot.h>

/* Initialize IRT object */
TowerRobot::IRT irt = TowerRobot::IRT(CONTROL_ADDRESS, 13, 5);

using namespace IRcommands;

void setup() {
  irt.begin();
  irt.synchronize(2, 500);
}

void loop() {
  
}