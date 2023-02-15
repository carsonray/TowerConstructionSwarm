// Include the TowerRobot Library
#include <TowerRobot.h>
#include <ScaledStepper.h>
#include <Button.h>

// Slide parameters
const double stepsPerBlock = -200.0/90*27;
const double upperLimit = 10;

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

//Creates color sensor instance
TowerRobot::ColorSensor colorSensor = TowerRobot::ColorSensor();

//Creates IRT instance
TowerRobot::IRT irt = TowerRobot::IRT(CONTROL_ADDRESS+2, 3, 5);

//Creates towerrobot instance
TowerRobot robot = TowerRobot(&slide, &turret, &gripper, &colorSensor, &irt);

void setup() {
  Serial.begin(9600);
  //robot.setTowerHeights(4, 0, 0, 0);
  irt.begin();
  //robot.home();
  robot.synchronize();
}

void loop() {
  gripper.close();
}