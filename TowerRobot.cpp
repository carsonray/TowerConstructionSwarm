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
    updateYield();

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
    //Begins yielding
    beginYield();

    //Corrects block number to be greater than tower height
    if (blockNum < towerHeights[tower]) {
      blockNum = towerHeights[tower];
    }
    
    //If not at correct tower
    if (turret->getTowerPos() != tower) {
      //Moves to clear current tower
      if (slide->currentPosition() - (towerHeights[turret->getTowerPos()] + slide->getClearMargin()) < -slide->getStepError()) {
        slide->moveToClear(towerHeights[turret->getTowerPos()]);
        while(slide->run()) {
          updateYield();
        }
      }

      //Loops through towers between current and target
      int testPos = turret->nextTowerTo(tower);
      while (true) {
        //If current position will not clear tower
        if (slide->currentPosition() - (towerHeights[testPos] + slide->getClearMargin()) < -slide->getStepError()) {
          //Moves to height of obstructing tower
          slide->moveToClear(towerHeights[testPos]);

          //Moves to carry position next to tower
          turret->moveToCarry(testPos);

          //Runs slide and turret
          bool slideRun = true;
          bool turretRun = true;
          while (slideRun || turretRun) {
            updateYield();

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
      while(turret->run()) {
        updateYield();
      }
    }

    //Moves to correct block position
    slide->moveToBlock(blockNum);
    while(slide->run()) {
      updateYield();
    }

    //Ends yielding
    endYield();
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
  if (colorInit) {
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
  } else {
    return -1;
  }
}

//Synchronizes so all robots start at the same time
void TowerRobot::synchronize() {
  if (irtInit) {
    irt->setAutoRelay(true);
    unsigned int command, data;
    while (true) {
      //Waits until data is received
      irt->waitReceive();
      irt->receive(&command, &data);

      //Delays response
      delay(DELAY_CYCLE);
      
      //Checks to ensure command is poll
      if (command == IR_POLL) {
        if  {
          //Sends ready status to controller
          irt->send(CONTROL_ADDRESS, IR_POLL, irt->getAddress());
        }
      } else if (command == IR_DONE) {
        //Stops if all robots ready
        break;
      }
      irt->update();
    }
    irt->setAutoRelay(false);
  }
}

//Sets whether movements are yielded to other robots
void TowerRobot::setYieldActive(bool active) {
  //If state is changed
  if (active != yieldEnabled) {
    yieldEnabled = active;
    if (!yieldEnabled) {
      //Stops polling signal
      endYield();
    }
  }
}

//Begins polling signal
void TowerRobot::beginYield() {
  if (irtInit && yieldEnabled) {
    yieldActive = true;

    //Resets closest tower
    closestTower = turret->closestTower();
    prevClosestTower = closestTower;

    irt->send(MASTER_ADDRESS, IR_POLL, irt->getAddress());
    irt->setSendInterval(100, 500);
    irt->setSendRepeats(-1);
  }
}

//Ends polling signal
void TowerRobot::endYield() {
  if (irtInit) {
    yieldActive = false;

    irt->setSendRepeats(0);
    irt->update();
  }
}

//Updates yield protocol
bool TowerRobot::updateYield() {
  bool isYielding = false;
  bool blocked = false;
  int address, command, data;

  while (irtInit && yieldActive) {
    //Checks to see whether data was received
    irt->update();
    if (irt->receive(&address, &command, &data)) {
      if (address == MASTER_ADDRESS) {
        //If command was not directed
        if ((command == IR_POLL) && (data < irt->getAddress())) {
          //If polling signal detected with subordinate address
          

          //Updates closest tower
          prevClosestTower = closestTower;
          closestTower = turret->closestTower();

          //Send tower information
          irt->send(data, IR_CLOSEST_TOWER, closestTower);
          irt->waitSend();
          irt->send(data, IR_UPDATE_HEIGHT, towerHeights[closestTower]);
          irt->waitSend();

          //Stops blocking previous
          if (prevClosestTower != closestTower) {
            irt->send(MASTER_ADDRESS, IR_DONE, prevClosestTower);
          }

          //Resumes polling signal
          beginYield();
        } else if ((command == IR_DONE) && (data == turret->closestTower())) {
          //Stops yielding if interferance at tower no longer exists
          isYielding = false;
        }
      } else if (address == irt->getAddress()) {
        //If command was directed
        if ((command == IR_CLOSEST_TOWER) && (data == turret->closestTower())) {
          //If interferance is detected
          isYielding = true;
          blocked = true;
        } else if (command == IR_UPDATE_HEIGHT) {
          //Updates tower height
          towerHeights[turret->closestTower()] = data;
        }
      }
    }

    //Resumes if no longer yielding
    if (!isYielding) {
      break;
    }
  }

  return blocked;
}