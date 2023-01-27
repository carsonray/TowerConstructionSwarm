#include <TowerRobot.h>

/* Initialize IRT object */
TowerRobot::IRT irt = TowerRobot::IRT(1, 3, 5);

void setup(void) {
  Serial.begin(9600);

  irt.begin();
}

void loop(void) {
  unsigned int command, data;
  if (irt.recieve(&command, &data)) {
    Serial.print("Command: ");
    Serial.println(command);
    Serial.print("Data: ");
    Serial.println(data);
  }
  irt.update();
  delay(50);
}