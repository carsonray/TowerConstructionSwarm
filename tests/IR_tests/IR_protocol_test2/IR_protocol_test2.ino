#include <TowerRobot.h>

/* Initialize IRT object */
TowerRobot::IRT irt = TowerRobot::IRT(2, 3, 5);

using namespace IRcommands;

void setup(void) {
  Serial.begin(9600);

  irt.begin();
}

void loop(void) {
  unsigned int command, data;
  if (irt.receive(&command, &data)) {
    Serial.print("Command: ");
    Serial.println(command);
    Serial.print("Data: ");
    Serial.println(data);
  }
  irt.update();
}