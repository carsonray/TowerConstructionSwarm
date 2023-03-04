#include <TowerRobot.h>
#include <Button.h>

/* Initialize IRT object */
TowerRobot::IRT irt = TowerRobot::IRT(CONTROL_ADDRESS, 13, 5);

//Target address
int address = 0;

//Joystick pins
#define xPin A0
#define yPin A1

//Joystick button
Button button = Button(2, true, 0);

//Slide position
int slidePos = 0;

//Turret position
int turretPos = 0;

//Gripper state
bool gripState = true;

using namespace IRcommands;

void setup() {
  pinMode(2, INPUT_PULLUP);
  irt.begin();
}

void loop() {
  //Checks for joystick commands
  if (analogRead(xPin) < 490) {
    slidePos++;
  } else if (analogRead(xPin) > 530) {
    slidePos--;
  }

  if (analogRead(yPin) < 490) {
    turretPos++;
    turretPos = turretPos % 4;
  } else if (analogRead(yPin) > 530) {
    turretPos--;
    turretPos = (turretPos + 4) % 4;
  }

  //Checks for button command
  if (button.changeTo(LOW)) {
    gripState = !gripState;
  }

  //Updates actions
  button.update();

  irt.send(CONTROL_ADDRESS+1+address, SLIDE, slidePos);
  irt.waitSend();
  irt.send(CONTROL_ADDRESS+1+address, TURRET, turretPos);
  irt.waitSend();
  irt.send(CONTROL_ADDRESS+1+address, GRIPPER, (int) gripState);
  irt.waitSend();

  delay(100);
}