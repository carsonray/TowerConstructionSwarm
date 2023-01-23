// Include the TowerRobot Library
#include <TowerRobot.h>
#include <ScaledStepper.h>

// Define parameters
const double stepsPerDegree = -200.0*142/32/360;

const int stepPin = 6;
const int dirPin = 7;
const int modePins[3] = {3, 4, 5};


int towerNum = 1;

// Creates scaled stepper
ScaledStepper stepper = ScaledStepper(stepPin, dirPin, modePins[0], modePins[1], modePins[2]);

// Creates a turret instance
TowerRobot::Turret turret = TowerRobot::Turret(stepsPerDegree, &stepper);

void setup() {
  
}

void loop() {
    turret.moveToTower(towerNum);
    turret.wait();
    delay(10000);
    towerNum = (towerNum + 1)%4;
}