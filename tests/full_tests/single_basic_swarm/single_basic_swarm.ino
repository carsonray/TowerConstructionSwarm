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

//Creates towerrobot instance
TowerRobot robot = TowerRobot(&slide, &turret, &gripper, &colorSensor);

//Target color
int targetColor = BLUE;

//Target tower
int targetTower = 0;

//Cargo limit
int cargoLimit = 3;



//Random tower to load from
int loadTower;

//Random tower to unload on
int unloadTower = -1;

//Current height of tower
int currHeight;

//Buffer array to fill with colors
int bufferColors[10];

//Array to show which towers are availiable
bool openTowers[4] = {true, true, true, true};

void setup() {
  robot.begin();
  robot.setTowerHeights(1, 1, 1, 1);
  robot.home();
}

void loop() {
  //Updates avialiable load towers
  for (int i = 0; i < 4; i++) {
    if (robot.getTowerHeight(i) == 0) {
      //Tower is not open if has no height
      openTowers[i] = false;
    }
  }

  //Gets load tower
  if (openTowers[targetTower]) {
    //Ensures target tower is fully unloaded
    loadTower = targetTower;
  } else {
    //Gets random availiable load tower
    while (true) {
      loadTower = random(0, 4);

      if (openTowers[loadTower]) {
        break;
      }
    }
  }

  //Updates tower height
  currHeight = robot.findHeight(loadTower, bufferColors);

  //Ends if height is zero
  if (currHeight == 0) {
    return;
  }
 
  bool startedTarget;
  int loadBlock = robot.scanTower(loadTower, targetColor, &startedTarget, bufferColors);
  
  //Ensures full towers are not moved unless it is non-target blocks off of the target tower or all target blocks on non-target tower
  if ((loadBlock != 0) || (startedTarget != (loadTower == targetTower))) {
    //Grabs higher up if cargo limit was reached
    if ((currHeight - loadBlock) > cargoLimit) {
      loadBlock = currHeight - cargoLimit;
    }
    
    //Loads block
    robot.load(loadTower, loadBlock);

    if (startedTarget && (loadTower != targetTower)) {
      //Ensures target blocks are unloaded on target tower
      unloadTower = targetTower;
    } else {
      //Unloads on random tower
      while (true) {
        unloadTower = random(0, 4);

        //Ensures uneccesary blocks are not unloaded on same load tower or target tower
        if ((unloadTower != loadTower) && (unloadTower != targetTower)) {
          break;
        }
      }
    }

    robot.unload(unloadTower);
  } else if (loadBlock == 0) {
    //Locks tower if nothing should be loaded
    openTowers[loadTower] = false;
  }
}