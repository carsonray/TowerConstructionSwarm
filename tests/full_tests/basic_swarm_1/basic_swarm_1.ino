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
int targetTowerPos = 0;

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
  robot.setTowerHeights(1, 1, 1, 1);
  robot.setYieldActive(true);
  robot.home();
  robot.synchronize();
}

void loop() {
  //Updates avialiable load towers
  bool availiable = false;
  for (int i = 0; i < 4; i++) {
    if (robot.getTowerHeight(i) == 0) {
      //Tower is not open if has no height
      openTowers[i] = false;
    }
    availiable = availiable || openTowers[i];
  }

  //If no availiable tower, ensures yield is being updated
  if (!availiable) {
    robot.beginYield();
    turret.moveToCarry(turret.closestTower());
    robot.waitSlideTurret();
    while (true) {
      robot.updateYield();
    }
  }

  //Gets load tower
  if (openTowers[targetTowerPos]) {
    //Ensures target tower is fully unloaded
    loadTower = targetTowerPos;
  } else {
    //Gets random availiable load tower
    while (true) {
      randomSeed(analogRead(A0));
      loadTower = random(0, 4);

      if (openTowers[loadTower]) {
        break;
      }
    }
  }
  
  //Gets predicted tower height
  currHeight = robot.getTowerHeight(loadTower);
  currBlock = currHeight;

  //Resets buffer color array
  for (int i = 0; i < 10; i++) {
    bufferColors[i] = -2;
  }
  
  //Finds actual tower height
  bool startedEmpty = false;
  while (currBlock >= 0) {
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

  //Updates tower height
  currHeight = robot.getTowerHeight(loadTower);
  currBlock = currHeight;

  //Ends if height is zero
  if (currHeight== 0) {
    return;
  }
 
  //Whether the top of the tower was the target color
  bool startedTarget = false;

  //Loops through checked colors and then finds more colors if necessary
  while (currBlock > 0) {
    //Moves down tower
    currBlock--;

    //If color is not read, reads it in
    checkColor = bufferColors[currBlock];
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
  if ((currBlock != 0) || (startedTarget != (loadTower == targetTowerPos))) {
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
      if (startedTarget && (loadTower != targetTowerPos)) {
        //Ensures target blocks are unloaded on target tower
        unloadTower = targetTowerPos;
      } else {
        //Unloads on random tower
        while (true) {
          randomSeed(analogRead(A0));
          unloadTower = random(0, 4);

          //Ensures uneccesary blocks are not unloaded on same load tower or target tower
          if ((unloadTower != loadTower) && (unloadTower != targetTowerPos)) {
            break;
          }
        }
      }

      if (robot.unload(unloadTower)) {
        break;
      }
    }
  } else if (currBlock == 0) {
    //Locks tower if nothing should be loaded
    openTowers[loadTower] = false;
  }
}