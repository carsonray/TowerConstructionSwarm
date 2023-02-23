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
TowerRobot::IRT irt = TowerRobot::IRT(CONTROL_ADDRESS+1, 3, 5);

//Creates towerrobot instance
TowerRobot robot = TowerRobot(&slide, &turret, &gripper, &colorSensor, &irt);

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

//Current block on tower
int currBlock;

//Current color of block
int checkColor;

//Buffer array to fill with colors
int bufferColors[10] = {-2, -2, -2, -2, -2, -2, -2, -2, -2, -2};

//Difference between buffer array and actual tower
int bufferDiff = 0;

//Array to show which towers are availiable
bool openTowers[4] = {true, true, true, true};

void setup() {
  /*randomSeed(analogRead(A0));
  robot.setTowerHeights(3, 3, 3, 3);
  robot.home();
  robot.synchronize();*/
  robot.setTowerHeights(4, 0, 0, 0);
  robot.setAutoRelay(true);
  robot.setYieldActive(true);
  robot.home();
  robot.synchronize();
  robot.load(0);
  robot.unload(1);
}

void loop() {
  /*
  if ((robot.getTowerHeight(targetTower) != 0) && openTowers[targetTower]) {
    //Ensures target tower is fully unloaded
    loadTower = targetTower;
  } else {
    //Gets random load tower that was not the previous unload tower
    //And has blocks on it
    //And is availiable
    while (true) {
      loadTower = random(0, 4);

      if ((loadTower != unloadTower) && (robot.getTowerHeight(loadTower) > 0) && openTowers[loadTower]) {
        break;
      }
    }
  }
  
  //Gets predicted tower height
  currHeight = robot.getTowerHeight(loadTower);
  currBlock = currHeight;
  */
  
  //Finds actual tower height
  /*bool startedEmpty = false;
  while (true) {
    checkColor = robot.scanBlock(loadTower, currBlock);

    //Checks whether the first check was empty
    if (currBlock == currHeight) {
      startedEmpty = (checkColor == EMPTY);
    }

    //Updates buffer array with color
    if (checkColor != EMPTY) {
      bufferColors[currBlock] = checkColor;
    }    
    
    if (startedEmpty) {
      //Moves down until tower is found
      if (checkColor != EMPTY) {
        break;
      }
      currBlock--;
    } else {
      //Moves up until no tower is found
      if (checkColor == EMPTY) {
        break;
      }
      currBlock++;
    }
  }

  //Gets offset of buffer array if height changed
  bufferDiff = robot.getTowerHeight(loadTower) - currHeight;

  //Updates tower height
  currHeight = robot.getTowerHeight(loadTower);
  currBlock = currHeight;
  */
 /*
  //Whether the top of the tower was the target color
  bool startedTarget = false;

  //Loops through checked colors and then finds more colors if necessary
  while (currBlock > 0) {
    //Moves down tower
    currBlock--;

    //If color is not read, reads it in
    checkColor = bufferColors[currBlock + bufferDiff];
    if (checkColor == -2) {
      checkColor = robot.scanBlock(loadTower, currBlock);
    }

    //Checks whether the top of the tower was the target color
    if (currBlock == (currHeight - 1)) {
      startedTarget = (checkColor == targetColor);
    }

    //Breaks if color changes away or to target color
    if (startedTarget != (checkColor == targetColor)) {
      //Moves to block before change
      currBlock++;
      break;
    }
  }

  //Ensures full towers are not moved unless it is non-target blocks off of the target tower or all target blocks on non-target tower
  if ((currBlock != 0) || (startedTarget != (loadTower == targetTower))) {
    //Grabs higher up if cargo limit was reached
    if ((currHeight - currBlock) > cargoLimit) {
      currBlock = currHeight - cargoLimit;
    }
    
    //Loads block if not blocked
    if (!robot.load(loadTower, currBlock)) {
      return;
    }

    //Loops until unloaded
    while (true) {
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

      if (robot.unload(unloadTower))) {
        break;
      }
    }
  } else if (currBlock == 0) {
    //Locks tower if nothing should be loaded
    openTowers[loadTower] = false;
  }
  */
}