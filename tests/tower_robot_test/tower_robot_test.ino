// Include the TowerRobot Library
#include <TowerRobot.h>
#include <ScaledStepper.h>
#include <Button.h>

// Slide parameters
const double stepsPerBlock = -200.0/90*27;
const double upperLimit = 6;

#define slideStep 12
#define slideDir 13
const int slideMode[3] = {9, 10, 11};

#define limitPin 8

// Creates scaled stepper
ScaledStepper slideStepper = ScaledStepper(slideStep, slideDir, slideMode[0], slideMode[1], slideMode[2]);

// Creates a limit switch
Button limit = Button(limitPin);

// Creates a slide instance
TowerRobot::Slide slide = TowerRobot::Slide(stepsPerBlock, upperLimit, &slideStepper, &limit);

// Turret parameters
const double stepsPerDegree = -200.0*142/32/360;

const int turretStep = 6;
const int turretDir = 7;
const int turretMode[3] = {1, 2, 4};

// Creates scaled stepper
ScaledStepper turretStepper = ScaledStepper(turretStep, turretDir, turretMode[0], turretMode[1], turretMode[2]);

// Creates a turret instance
TowerRobot::Turret turret = TowerRobot::Turret(stepsPerDegree, &turretStepper);

//Gripper parameters
#define gripPin 0

//Creates gripper instance
TowerRobot::Gripper gripper = TowerRobot::Gripper(gripPin);

//Creates towerrobot instance
TowerRobot robot = TowerRobot(&slide, &turret, &gripper);

int targetTower = 2;

void setup() {
  robot.setTowerHeights(1, 1, 0, 0);
  robot.home();

  robot.load(0);

  //Moves cargo to adjacent tower
  int cargo = robot.getCargo();
  int adjacentTower = turret.nextTower(targetTower, -1);
  robot.unload(adjacentTower);

  //Updates height of target tower
  int height = robot.findHeight(targetTower);

  //Reloads cargo
  robot.load(adjacentTower, robot.getTowerHeight(adjacentTower) - cargo);
}

void loop() {
  
}