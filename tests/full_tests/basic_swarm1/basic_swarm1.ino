// Include the TowerRobot Library
#include <TowerRobot.h>
#include <ScaledStepper.h>
#include <Button.h>

// Slide parameters
const double stepsPerBlock = -200.0/90*28;
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
int targetColor = 3;

//Target tower
int targetTower = 0;

//Cargo limit
int cargoLimit = 3;

//Random tower to load from
int loadTower;

//Random tower to unload on
int unloadTower;

//Current block on tower
int currBlock;

//Current color of block
int checkColor;

//Buffer array to fill with colors
int bufferColors[10];

//Array to show which towers are availiable
bool openTowers[4] = {true, true, true, true};

void setup() {
  randomSeed(analogRead(A0));
  robot.setTowerHeights(3, 3, 3, 3);
  robot.home();
  //robot.synchronize();
}

void loop() {
  //Gets new load tower that was not the previous unload tower
  //And has blocks on it
  //And is availiable
  while (true) {
    loadTower = random(0, 4);

    if ((loadTower != unloadTower) && (robot.getTowerHeight(loadTower) > 0) && openTowers[loadTower]) {
      break;
    }
  }
  
  //Finds top of tower

  //Current index of buffer array
  int bufferIndex = -1;

  //Current block on tower
  currBlock = robot.getTowerHeight(loadTower);
  bool startedEmpty = false;
  checkColor = -1;
  while (true) {
    checkColor = robot.scanBlock(loadTower, currBlock);

    //Checks whether the first check was empty
    if (bufferIndex == -1) {
      startedEmpty = (checkColor == -1);
      bufferIndex++;
    }
    
    if (startedEmpty) {
      if (checkColor != -1) {
        //If tower is found, add check to buffer and exit
        bufferColors[bufferIndex] = checkColor;
        bufferIndex++;
        break;
      }
      //Moves down until tower is found
      currBlock--;
    } else {
      if (checkColor != -1) {
        //Fills buffer color array until check is empty
        bufferColors[bufferIndex] = checkColor;
        bufferIndex++;
      } else {
        //Exits if check is empty
        break;
      }
      //Moves up until no tower is found
      currBlock++;
    }
  }

  //Loops through checked colors and then finds more colors if necessary
  
  //Whether the top of the tower was the target color
  bool startedTarget = false;

  int currHeight = robot.getTowerHeight(loadTower);
  currBlock = currHeight - 1;
  while (currBlock >= 0) {
    //If color is not read, reads it in
    if (currBlock < (currHeight - bufferIndex)) {
      checkColor = robot.scanBlock(loadTower, currBlock);
    } else {
      checkColor = bufferColors[currBlock - (currHeight - bufferIndex)];
    }

    //Checks whether the top of the tower was the target color
    if (currBlock == (currHeight - 1)) {
      startedTarget = (checkColor == targetColor);
    }

    //Breaks if color changes away or to target color
    if ((startedTarget && (checkColor != targetColor)) || (!startedTarget && (checkColor == targetColor))) {
      //Moves to position before change
      currBlock++;
      break;
    }

    currBlock--;
  }

  //Ensures full towers are not moved unless it is non-target blocks off of the target tower or all target blocks on non-target tower
  if ((currBlock != 0) || (startedTarget && (loadTower != targetTower)) || (!startedTarget && (loadTower == targetTower))) {
    //Loads block
    robot.load(loadTower, currBlock);

    //Grabs higher up if cargo limit was reached
    if ((currHeight - currBlock) > cargoLimit) {
      currBlock = currHeight - cargoLimit;
    }

    if (startedTarget && (robot.getTowerHeight(targetTower) == 0)) {
      //Ensures target blocks are loaded on target tower when ready
      unloadTower = targetTower;
    } else {
      //Unloads on random tower
      while (true) {
        unloadTower = random(0, 4);

        //Ensures non-target blocks are not loaded on target tower
        //And ensures blocks are not unloaded on same tower they are loaded on
        if ((unloadTower != loadTower) && (startedTarget || (unloadTower != targetTower))) {
          break;
        }
      }
    }

    robot.unload(unloadTower);
  } else {
    //Locks tower
    openTowers[loadTower] = false;
  }
}