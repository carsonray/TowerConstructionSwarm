#include <TowerRobot.h>
#include <Button.h>

/* Initialize IRT object */
TowerRobot::IRT irt = TowerRobot::IRT(CONTROL_ADDRESS, 13, 5);

//Joystick pins
const int xPins[2] = {A0, A2};
const int yPins[2] = {A1, A3};

//Joystick buttons
Button buttons[2] = {Button(2), Button(3)};

//Slide positions
int slidePos[2] = {0, 0};

//Turret positions
int turretPos[2] = {0, 0};

//Gripper states
bool gripState[2] = {true, true};

using namespace IRcommands;

void setup() {
  irt.begin();
}

void loop() {
  for (int i = 0; i < 2; i++) {
    //Checks for joystick commands
    if (analogRead(yPins[i]) > 520) {
      slidePos[i]++;
    } else if (analogRead(yPins[i] < 504)) {
      slidePos[i]--;
    }

    if (analogRead(xPins[i]) > 520) {
      turretPos[i]++;
      turretPos[i] = turretPos[i] % 4;
    } else if (analogRead(xPins[i]) < 504) {
      turretPos[i]--;
      turretPos[i] = (turretPos[i] + 4) % 4;
    }

    //Checks for button command
    if (buttons[i].changeTo(LOW)) {
      gripState[i] = !gripState[i];
    }

    //Updates actions
    irt->send(CONTROL_ADDRESS+1+i, SLIDE, slidePos[i]);
    irt->waitSend();
    irt->send(CONTROL_ADDRESS+1+i, TURRET, turretPos[i]);
    irt->waitSend();
    irt->send(CONTROL_ADDRESS+1+i, GRIPPER, (int) gripState[i]);
    irt->waitSend();

    buttons[i].update();

    delay(DELAY_CYCLE);
  }
}