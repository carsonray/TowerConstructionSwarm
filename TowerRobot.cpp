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
  slide->home(homePos);
}

bool TowerRobot::waitSlideTurret() {
  bool blocked = false;

  bool slideRun = true;
  bool turretRun = true;
  while (slideRun || turretRun) {
    //If yielding was blocked
    if (!updateYield()) {
      blocked = true;

      //Aborts if gripper closed
      if (!gripper->isOpen()) {
        break;
      }
    }

    slideRun = slide->run();
    turretRun = turret->run();
  }

  return !blocked;
}

//Moves to tower and block position
bool TowerRobot::moveToBlock(int tower) {
  //Moves to top of tower as default
  return moveToBlock(tower, towerHeights[tower] - 1);
}
bool TowerRobot::moveToBlock(int tower, double blockNum) {
  if (cargo == 0) {
    //No cargo

    //Opens gripper to clear towers
    gripper->open();

    //Moves to correct position
    slide->moveToBlock(blockNum);
    turret->moveToTower(tower);
    if (!waitSlideTurret()) {
      return false;
    }
  } else {
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
          if (!updateYield()) {
            return false;
          }
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
            if (!updateYield()) {
              return false;
            }

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
        if (!updateYield()) {
          return false;
        }
      }
    }

    //Moves to correct block position
    slide->moveToBlock(blockNum);
    while(slide->run()) {
      if (!updateYield()) {
        return false;
      }
    }
  }

  return true;
}

//Loads block(s) from position on tower
bool TowerRobot::load(int tower) {
  //Loads from top of tower as default
  return load(tower, towerHeights[tower] - 1);
}
bool TowerRobot::load(int tower, int blockNum) {
  //Moves to correct tower and block
  if (blockNum >= 0) {
    //Begins yielding
    beginYield();

    if (!moveToBlock(tower, blockNum)) {
      return false;
    }

    //Closes gripper
    gripper->close();
    gripper->wait();

    //Updates tower height and cargo
    cargo = towerHeights[tower] - blockNum;
    towerHeights[tower] -= cargo;
  }

  return true;
}

//Unloads block(s) on top of tower
bool TowerRobot::unload(int tower) {
  if (cargo > 0) {
    //Begins yielding
    beginYield();

    //Moves one block above top of tower
    if (!moveToBlock(tower, towerHeights[tower])) {
      return false;
    }

    //Opens gripper
    gripper->open();
    gripper->wait();

    //Ends yielding
    endYield();

    //Updates tower height and cargo
    towerHeights[tower] += cargo;
    cargo = 0;
  }

  return true;
}

//Scans color of particular block
int TowerRobot::scanBlock(int tower, int blockNum) {
  if (colorInit) {
    //Moves to tower clockwise from target to align color sensor with target
    if (!moveToBlock(turret->nextTower(tower, -1), blockNum + sensorMargin)) {
      return -2;
    }

    //Gets color of block
    int blockColor = colorSensor->getBlockColor();

    //Updates tower height
    if ((blockColor > EMPTY) && (blockNum >= towerHeights[tower])) {
      towerHeights[tower] = blockNum + 1;
    } else if ((blockColor == EMPTY) && (blockNum < towerHeights[tower])) {
      towerHeights[tower] = blockNum;
    }

    return blockColor;
  } else {
    return EMPTY;
  }
}

//Synchronizes so all robots start at the same time
void TowerRobot::synchronize() {
  if (irtInit) {
    unsigned int command, data;
    while (true) {
      //Waits until done is received
      irt->waitReceive();
      irt->receive(&command, &data);

      if (command == DONE) {
        //Relays done signal
        delay(DELAY_CYCLE);
        irt->send(MASTER_ADDRESS, DONE, data);
        irt->waitSend();
        break;
      }
    }
  }
}

//Sets whether undirected commands are automatically relayed
void TowerRobot::setAutoRelay(bool active) {
  irt->setAutoRelay(active);
}

//Sets whether movements are yielded to other robots
void TowerRobot::setYieldActive(bool active) {
  yieldActive = active;
  if (!yieldActive) {
    //Sets dormant mode
    yieldMode = DORMANT;
  }
}

//Begins polling signal
void TowerRobot::beginYield() {
  if (irtInit && yieldActive) {
    //Activates polling mode
    if (yieldMode == DORMANT) {
      yieldMode = POLLING;
    }

    //Resets closest tower
    closestTower = turret->closestTower();
  }
}

//Ends polling signal
void TowerRobot::endYield() {
  if (irtInit && yieldActive) {
    //Activates dormant mode
    yieldMode = DORMANT;

    //Sends done signal
    irt->send(MASTER_ADDRESS, DONE, turret->closestTower());
    irt->waitSend();
  }
}

//Updates yield protocol
bool TowerRobot::updateYield() {
  bool blocked = false;
  unsigned int command, data;

  while (irtInit && (yieldMode != DORMANT)) {
    //Stops blocking previous
    if (closestTower != turret->closestTower()) {
      irt->send(MASTER_ADDRESS, DONE, closestTower);
      irt->waitSend();

      //Resets closest tower
      closestTower = turret->closestTower();
    }

    //Checks to see whether data was received
    irt->update();
    if (irt->receive(&command, &data)) {
      if (command == POLL) {
        if (data < irt->getAddress()) {
          //If polling signal detected with subordinate address
          //Activates update mode
          if (yieldMode == POLLING) {
            yieldMode = TOWER_UPDATING;
          }
        } else {
          //If polling signal detected with superior address
          //Activates pending mode
          yieldMode = PENDING;
        }
      } else if ((command == DONE) && (data == turret->closestTower())) {
        //Resumes polling when no interferance
        yieldMode = POLLING;
      }

      if (yieldMode == PENDING) {
        //Updating commands
        if (command == CLOSEST_TOWER) {
          if (data == turret->closestTower()) {
            //If interferance is detected
            yieldMode = BLOCKED;
            blocked = true;

            //If gripper is closed, moves to carry position to avoid interference
            if (!gripper->isOpen()) {
              turret->moveToCarry(turret->closestTower());
              turret->wait();
            }
          } else {
            //Resumes polling when no interferance
            yieldMode = POLLING;
          }
        } else if (command == UPDATE_HEIGHT) {
          //Updates tower height
          towerHeights[turret->closestTower()] = data;
        }
      }
    }

    //Updates polling modes
    if ((yieldMode > PENDING) && (!irt->isSending())) {
      if (yieldMode == POLLING) {
        //Sends polling signal
        irt->send(MASTER_ADDRESS, POLL, irt->getAddress());
        irt->setSendInterval(200, 500);
      } else if (yieldMode == TOWER_UPDATING) {
        //Sends closest tower
        irt->send(MASTER_ADDRESS, CLOSEST_TOWER, closestTower);
        irt->setSendInterval(200, 500);
        
        //Switches to next mode
        yieldMode = HEIGHT_UPDATING;
      } else if (yieldMode == HEIGHT_UPDATING) {
        //Updates tower height
        irt->send(MASTER_ADDRESS, UPDATE_HEIGHT, towerHeights[closestTower]);
        irt->setSendInterval(200, 500);

        //Switches to next mode
        yieldMode = TOWER_UPDATING;
      }

      //Ensures interval is waited
      irt->useInterval();
    }

    //Does not block other tasks if gripper is open
    if (gripper->isOpen() || (yieldMode != BLOCKED)) {
      break;
    }
  }

  return !blocked;
}

void TowerRobot::remoteControl() {
  if (irtInit) {
    irt->update();
    unsigned int command, data;
    if (irt->receive(&command, &data)) {
      if (command == SLIDE) {
        slide->moveToBlock(data);
      } else if (command == TURRET) {
        turret->moveToTower(data);
      } else if (command == CARRY) {
        turret->moveToCarry(data);
      } else if (command == GRIPPER) {
        if (data == 0) {
          gripper->open();
        } else {
          gripper->close();
        }
      }
    }
  }
}