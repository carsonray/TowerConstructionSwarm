// Include the TowerRobot Library
#include <TowerRobot.h>
#include <ScaledStepper.h>

// Define parameters
const double stepsPerDegree = -200.0*142/32/360;

const int stepPin = 6;
const int dirPin = 7;
const int modePins[3] = {1, 2, 4};

//Target tower position
int towerNum = 0;

//Direction of movement
int dir = 1;

// Creates scaled stepper
ScaledStepper stepper = ScaledStepper(stepPin, dirPin, modePins[0], modePins[1], modePins[2]);

// Creates a turret instance
TowerRobot::Turret turret = TowerRobot::Turret(stepsPerDegree, &stepper);

void setup() {
  Serial.begin(9600);
}

void loop() {
    //Moves to next tower
    turret.moveToTower(towerNum);
    turret.wait();
    delay(5000);

    //Changes direction if limit is reached
    if ((dir > 0) && (towerNum == 3)) {
      dir = -1;
    } else if ((dir < 0) && (towerNum == 0)) {
      dir = 1;
    }

    //Increments tower target in direction
    towerNum+=dir;
}