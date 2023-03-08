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

//Sleeps while updating ir
void TowerRobot::sleep(unsigned long timeout) {
  unsigned long timeoutStart = millis();
  while ((millis() - timeoutStart) < timeout) {
    if (irtInit) {
      irt->update();
    }
  }
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

    //If needs to move to different tower
    if (irtInit && (tower != turret->targetTower())) {
      //Moves slide to zero to avoid unloading robots
      slide->moveToBlock(0);
      turret->moveToCarry(turret->nextTowerTo(tower));
      bool slideRun = true;
      bool turretRun = true;
      while (slideRun || turretRun) {
        if (!updateYield()) {
          return false;
        }

        //Runs slide and turret
        slideRun = slide->run();
        turretRun = turret->run();

        //Moves turret to final position if slide is down all the way
        if (!slideRun) {
          turret->moveToTower(tower);

          //Moves slide to final position if turret is close enough
          if (turret->closestTower() == tower) {
            slide->moveToBlock(blockNum);
          }
        }
      }
    } else {
      //Moves to correct position
      slide->moveToBlock(blockNum);
      turret->moveToTower(tower);
      if (!waitSlideTurret()) {
        return false;
      }
    }
  } else {
    //Corrects block number to be greater than tower height
    if (blockNum < towerHeights[tower]) {
      blockNum = towerHeights[tower];
    }
    
    //If not at correct tower
    if (turret->targetTower() != tower) {
      int clearHeight;
      if (slide->currentPosition() - (clearHeight + slide->getClearMargin()) < -slide->getStepError()) {
        clearHeight = towerHeights[turret->targetTower()];
      } else {
        clearHeight = slide->targetBlock();
      }
        
      //Moves to clear current tower
      if (irtInit) {
        //Ensures unloading robots are staggered
        if (clearHeight % 2 != irt->getAddress() % 2) {
          clearHeight += 1;
        }

        //Ensures unloading robots are not at zero position to run into loading robots
        if (clearHeight == 0) {
          clearHeight = 2;
        }
      }
      
      //Moves to position and clears tower if necessary
      if (clearHeight == towerHeights[turret->targetTower()]) {
        slide->moveToClear(clearHeight);
      } else {
        slide->moveToBlock(clearHeight);
      }
      if (!waitSlideTurret()) {
        return false;
      }

      //Loops through towers between current and target
      int testPos = turret->nextTowerTo(tower);
      while (true) {
        //If current position will not clear tower
        if (slide->currentPosition() - (towerHeights[testPos] + slide->getClearMargin()) < -slide->getStepError()) {
          //Moves to height of obstructing tower
          clearHeight = towerHeights[testPos];

          //Ensures unloading robots are staggered
          if (irtInit && (clearHeight % 2 != irt->getAddress() % 2)) {
            clearHeight += 1;
          }

          //Moves to position and clears tower if necessary
          if (clearHeight == towerHeights[testPos]) {
            slide->moveToClear(clearHeight);
          } else {
            slide->moveToBlock(clearHeight);
          }

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
        testPos = turret->nextTowerTo(testPos, tower);
      }

      //Rotates final step to tower
      turret->moveToTower(tower);
      if (!waitSlideTurret()) {
        return false;
      }
    }

    //Moves to correct block position
    slide->moveToBlock(blockNum);
    if (!waitSlideTurret()) {
      return false;
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

    //Sends yielding signal
    sendYield();

    sleep(IR_CYCLE*2);

    if (!updateYield()) {
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

    //Updates tower height
    sendTowerUpdate();

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
    //Opens gripper to clear towers
    gripper->open();

    //Begins yielding
    beginYield();

    //Moves to tower clockwise from target to align color sensor with target
    if (!moveToBlock(turret->nextTower(tower, -1), blockNum + sensorMargin)) {
      return -2;
    }

    //Gets color of block
    if (irtInit) {
      irt->waitSync(2, COLOR_CYCLE);
    }

    int blockColor = colorSensor->getBlockColor();

    //Updates tower height
    if ((blockColor > EMPTY) && (blockNum >= towerHeights[tower])) {
      towerHeights[tower] = blockNum + 1;
    } else if ((blockColor == EMPTY) && (blockNum < towerHeights[tower])) {
      towerHeights[tower] = blockNum;
    }

    endYield();

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
        //Starts synchronization
        irt->beginSync();

        //Relays done signal
        sleep(IR_CYCLE);
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
    //Activates pending mode
    if (yieldMode == DORMANT) {
      yieldMode = PENDING;
    }

    //Updates turret angle
    turretAngle = Utils::modulo(turret->currentPosition(), 90.0);
  }
}

//Ends polling signal
void TowerRobot::endYield() {
  if (irtInit && yieldActive) {
    //Activates dormant mode
    yieldMode = DORMANT;
  }
}

//Sends yielding signal
void TowerRobot::sendYield() {
  if (irtInit) {
    irt->waitSync(2, IR_CYCLE);
    //Chooses loading or unloading command
    int command;
    if (cargo == 0) {
      command = LOADING;
    } else {
      command = UNLOADING;
    }
    //Sends yield with address and next tower
    int nextTower = turret->nextTowerTo(turret->targetTower());
    irt->send(MASTER_ADDRESS, command, irt->getAddress()*4 + nextTower);
    irt->waitSend();

    //Updates tower height
    sendTowerUpdate();
  }
}

//Sends tower update signal
void TowerRobot::sendTowerUpdate() {
  if (irtInit) {
    irt->waitSync(2, IR_CYCLE);
    //Updates tower height
    irt->send(MASTER_ADDRESS, TOWER_HEIGHT, towerHeights[nextTower]*4 + nextTower);
    irt->waitSend();
  }
}

//Updates yield protocol
bool TowerRobot::updateYield() {
  bool blocked = false;
  unsigned int command, data;

  if (irtInit && (yieldMode != DORMANT)) {
    //Gets new turret angle
    double newAngle = Utils::modulo(turret->currentPosition(), 90.0);

    //Gets direction of movement
    int dir = Utils::sign(turret->distanceToGo());

    //Uses corresponding send angle if direction is negative
    double useSendAngle = sendAngle;
    if (dir < 0) {
      useSendAngle = 90 - sendAngle;
    }

    //If angle has passed send threshold
    if ((turretAngle*dir < useSendAngle*dir) && (newAngle*dir > useSendAngle*dir)) {
      //Sends yield signal
      sendYield();
    }

    //Updates turret angle
    turretAngle = newAngle;

    //Loops until not blocked
    while (true) {
      //Updates signals
      irt->update();
      if (irt->receive(&command, &data)) {
        //Updates channel synchronization
        irt->updateSync(irt->getTimestamp(), IR_CYCLE);

        if (yieldMode == PENDING) {
          //If next tower matches
          if (data % 4 == turret->nextTowerTo(turret->targetTower())) {
            if (command == TOWER_HEIGHT) {
              //Updates tower height
              towerHeights[turret->nextTowerTo(turret->targetTower())] = data / 4;
            } else if ((turret->targetTower() == turret->closestTower()) || (cargo > 0) && (command == UNLOADING)) {
              //If targets match or both are unloading
              if ((data / 4) % 2 == 0) {
                //Blocks movement when superior address is interfering on same tower
                yieldMode = BLOCKED;
                blocked = true;
              } else {
                //Ensures inferior address is blocked
                irt->waitSync(2, IR_CYCLE);
                sendYield();
              }
            }
          }
        } else if ((yieldMode == BLOCKED) && (data % 4 != turret->targetTower())) {
          //Resumes pending when interference no longer exists
          yieldMode = PENDING;
        }
      }

      if (yieldMode != BLOCKED) {
        break;
      }
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