#include <TowerRobot.h>
#include <Button.h>

/* Initialize IRT object */
TowerRobot::IRT irt = TowerRobot::IRT(CONTROL_ADDRESS, 13, 5);

//Joystick pins
#define A0 xPin
#define A1 yPin

//Joystick button
Button click = Button(2);

//Slide position
int slidePos = 0;

//Turret position
int turretPos = 0;

//Gripper state
bool gripState = true;

using namespace IRcommands;

void setup() {
  irt.begin();
}

void loop() {
  //Checks for joystick commands
  if (analogRead(yPin) > 520) {
    slidePos++;
  } else if (analogRead(yPin < 504)) {
    slidePos--;
  }

  if (analogRead(xPin) > 520) {
    turretPos++;
    turretPos = turretPos % turret->numPos();
  } else if (analogRead(xPin) < 504) {
    turretPos--;
    turretPos = (turretPos + turret->numPos()) % turret->numPos();
  }

  //Checks for button command
  if (click.changeTo(LOW)) {
    gripState = !gripState;
  }

  //Updates actions
  irt->send(CONTROL_ADDRESS+1, IR_SLIDE, slidePos);
  irt->waitSend();
  irt->send(CONTROL_ADDRESS+1, IR_TURRET, turretPos);
  irt->waitSend();
  irt->send(CONTROL_ADDRESS+1, IR_GRIPPER, (int) gripState);
  irt->waitSend();

  click.update();

  delay(DELAY_CYCLE);
}