/*
  TowerRobot.cpp - Controls components of robot
  Created by Carson G. Ray, January 4 2022.
*/

#include <Arduino.h>
#include "TowerRobot.h"

TowerRobot::TowerRobot(Slide* slide, Turret* turret, Gripper* gripper) {
	this->slide = slide;
  this->turret = turret;
  this->gripper = gripper;
}

TowerRobot::TowerRobot(Slide* slide, Turret* turret, Gripper* gripper, ColorSensor* colorSensor) : TowerRobot(slide, turret, gripper) {
  this->colorSensor = colorSensor;

  colorInit = true;
}

TowerRobot::TowerRobot(Slide* slide, Turret* turret, Gripper* gripper, ColorSensor* colorSensor, IRT* irt) : TowerRobot(slide, turret, gripper, colorSensor) {
  this->irt = irt;

  irtInit = true;
}


//Sets tower heights
void TowerRobot::setTowerHeights(int tower1, int tower2, int tower3, int tower4) {
  towerHeights[0] = tower1;
  towerHeights[1] = tower2;
  towerHeights[2] = tower3;
  towerHeights[3] = tower4;
}

//Gets the height of a tower
int TowerRobot::getTowerHeight(int tower) {
  return towerHeights[tower];
}

//Homes robot
void TowerRobot::home() {
  home(slide->getHomePos());
}
void TowerRobot::home(double homePos) {
  if (irtInit) {
    irt->begin();
  }
  if (colorInit) {
    colorSensor->begin();
  }
  gripper->begin();
  gripper->open();

  turret->home();
  turret->moveTo(true, 0);
  turret->wait();

  slide->home(homePos);
}

void TowerRobot::waitSlideTurret() {
  bool slideRun = true;
  bool turretRun = true;
  while (slideRun || turretRun) {
    slideRun = slide->run();
    turretRun = turret->run();
  }
}

//Moves to tower and block position
void TowerRobot::moveToBlock(int tower) {
  //Moves to top of tower as default
  moveToBlock(tower, towerHeights[tower] - 1);
}
void TowerRobot::moveToBlock(int tower, double blockNum) {
  if (cargo == 0) {
    //No cargo

    //Opens gripper to clear towers
    gripper->open();

    //Moves to correct position
    slide->moveToBlock(blockNum);
    turret->moveToTower(tower);
    waitSlideTurret();
  } else {
    //Corrects block number to be greater than tower height
    if (blockNum < towerHeights[tower]) {
      blockNum = towerHeights[tower];
    }
    
    //If not at correct tower
    if (turret->getTowerPos() != tower) {
      //Moves to clear current tower
      if (slide->currentPosition() - (towerHeights[turret->getTowerPos()] + slide->getClearMargin()) < -0.01) {
        slide->moveToClear(towerHeights[turret->getTowerPos()]);
        slide->wait();
      }

      //Loops through towers between current and target
      int testPos = turret->nextTowerTo(tower);
      while (true) {
        //If current position will not clear tower
        if (slide->currentPosition() - (towerHeights[testPos] + slide->getClearMargin()) < -0.01) {
          //Moves to height of obstructing tower
          slide->moveToClear(towerHeights[testPos]);

          //Moves to carry position next to tower
          turret->moveToCarry(testPos);

          //Runs slide and turret
          bool slideRun = true;
          bool turretRun = true;
          while (slideRun || turretRun) {
            slideRun = slide->run();
            turretRun = turret->run();

            //Moves turret to final position if slide is done
            if (!slideRun) {
              turret->moveToTower(testPos);
            }
          }
        }

        //Ends if final tower was checked
        if (testPos == tower) {
          break;
        }

        //Moves to next tower position
        testPos = turret->nextTowerTo(tower);
      }

      //Rotates final step to tower
      turret->moveToTower(tower);
      turret->wait();
    }

    //Moves to correct block position
    slide->moveToBlock(blockNum);
    slide->wait();
  }
}

//Loads block(s) from position on tower
void TowerRobot::load(int tower) {
  //Loads from top of tower as default
  load(tower, towerHeights[tower] - 1);
}
void TowerRobot::load(int tower, int blockNum) {
  //Moves to correct tower and block
  if (blockNum >= 0) {
    moveToBlock(tower, blockNum);

    //Closes gripper
    gripper->close();
    gripper->wait();

    //Updates tower height and cargo
    cargo = towerHeights[tower] - blockNum;
    towerHeights[tower] -= cargo;
  }
}

//Unloads block(s) on top of tower
void TowerRobot::unload(int tower) {
  if (cargo > 0) {
    //Moves one block above top of tower
    moveToBlock(tower, towerHeights[tower]);

    //Opens gripper
    gripper->open();
    gripper->wait();

    //Updates tower height and cargo
    towerHeights[tower] += cargo;
    cargo = 0;
  }
}

//Scans color of particular block
int TowerRobot::scanBlock(int tower, int blockNum) {
  if (true/*colorInit*/) {
    //Moves to tower clockwise from target to align color sensor with target
    moveToBlock(turret->nextTower(tower, -1), blockNum + sensorMargin);

    //Gets color of block
    int blockColor = colorSensor->getBlockColor();

    //Updates tower height
    if ((blockColor > -1) && (blockNum >= towerHeights[tower])) {
      towerHeights[tower] = blockNum + 1;
    } else if ((blockColor == -1) && (blockNum < towerHeights[tower])) {
      towerHeights[tower] = blockNum;
    }

    return blockColor;
  }
}

//Synchronizes so all robots start at the same time
void TowerRobot::synchronize() {
  unsigned int command, data;
  while (true) {
    //Waits until data is received
    irt->waitReceive();
    irt->receive(&command, &data);

    //Delays response
    delay(DELAY_CYCLE);
    
    //Checks to ensure command is status related
    if (command == IR_STATUS) {
      if (data == IR_STATUS_POLL) {
        //Sends ready status to controller
        irt->send(CONTROL_ADDRESS, IR_STATUS, IR_STATUS_READY);
      } else if (data == IR_STATUS_READY) {
        //Ends synchronization if controller sends ready signal
        break;
      }
    }
    irt->update();
  }
}