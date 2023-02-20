#include <TowerRobot.h>

/* Initialize IRT object */
TowerRobot::IRT irt = TowerRobot::IRT(1, 3, 5);

using namespace IRcommands;

void setup() {
  Serial.begin(9600);

  irt.begin();
}

void loop() {
  unsigned int command, data;

  irt.send(0x2, 0x1, 0xAA);

  irt.setSendInterval(500);
  irt.setSendRepeats(5);
  irt.waitSend();

  delay(1000);

  irt.send(0x1, 0x2, 0xBB);

  irt.setSendInterval(500);
  irt.setSendRepeats(5);
  irt.waitSend();

  irt.waitReceive();
  irt.receive(&command, &data);
  Serial.print("Command: ");
  Serial.println(command, HEX);
  Serial.print("Data: ");
  Serial.println(data, HEX);

  delay(1000);
}