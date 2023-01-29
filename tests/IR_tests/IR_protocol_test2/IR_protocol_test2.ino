#include <TowerRobot.h>

/* Initialize IRT object */
TowerRobot::IRT irt = TowerRobot::IRT(2, 3, 5);

using namespace IRcommands;

void setup() {
  Serial.begin(9600);

  irt.begin();
}

void loop() {
  unsigned int command, data;
  if (irt.receive(&command, &data)) {
    Serial.print("Command: ");
    Serial.println(command, HEX);
    Serial.print("Data: ");
    Serial.println(data, HEX);
  }
  irt.update();
}