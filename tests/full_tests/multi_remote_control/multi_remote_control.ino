#include <TowerRobot.h>
#include <Button.h>

/* Initialize IRT object */
TowerRobot::IRT irt = TowerRobot::IRT(CONTROL_ADDRESS, 13, 5);

//Joystick pins
const int xPins[2] = {A0, A2};
const int yPins[2] = {A1, A3};

//Joystick buttons
int buttonPins[2] = {2, 3};
Button buttons[2] = {Button(2, true, 0), Button(3, true, 1)};

//Slide positions
int slidePos[2] = {0, 0};

//Turret positions
int turretPos[2] = {0, 0};

//Gripper states
bool gripState[2] = {true, true};

using namespace IRcommands;

void setup() {
  for (int i = 0; i < 2; i++) {
    pinMode(buttonPins[i], INPUT_PULLUP);
  }
  irt.begin();
}

void loop() {
  for (int i = 0; i < 2; i++) {
    if (analogRead(xPins[i]) < 490) {
      slidePos[i]++;
    } else if (analogRead(xPins[i]) > 530) {
      slidePos[i]--;
    }

    if (analogRead(yPins[i]) < 490) {
      turretPos[i]++;
      turretPos[i] = turretPos[i] % 4;
    } else if (analogRead(yPins[i]) > 530) {
      turretPos[i]--;
      turretPos[i] = (turretPos[i] + 4) % 4;
    }

    //Checks for button command
    if (buttons[i].changeTo(LOW)) {
      gripState[i] = !gripState[i];
    }

    //Updates actions
    buttons[i].update();

    irt.send(CONTROL_ADDRESS+1+i, SLIDE, slidePos[i]);
    irt.waitSend();
    irt.send(CONTROL_ADDRESS+1+i, TURRET, turretPos[i]);
    irt.waitSend();
    irt.send(CONTROL_ADDRESS+1+i, GRIPPER, (int) gripState[i]);
    irt.waitSend();

    delay(DELAY_CYCLE);
  }
}