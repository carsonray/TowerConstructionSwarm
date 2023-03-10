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

//Begins all components
void TowerRobot::begin() {
  if (irtInit) {
    irt->begin();
  }
  if (colorInit) {
    colorSensor->begin();
  }
  gripper->begin();
}

//Homes robot
void TowerRobot::home() {
  home(slide->getHomePos());
}
void TowerRobot::home(double homePos) {
  gripper->open();

  turret->home();
  slide->home(homePos);
}

bool TowerRobot::waitSlideTurret() {
  bool slideRun = true;
  bool turretRun = true;
  while (slideRun || turretRun) {
    //If yielding was blocked
    if (!updateYield()) {
      return false;
    }

    slideRun = slide->run();
    turretRun = turret->run();
  }

  return true;
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
    if (tower != turret->targetTower()) {
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

    //Sends yield signal
    sendYield();

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
  if (cargo == 0) {
    //Ensures block is not negative
    if (blockNum < 0) {
      blockNum = 0;
    }

    //Moves to correct tower and block
    setTurretTarget(tower);
    setSlideTarget(blockNum);
    if (!moveToBlock(tower, blockNum)) {
      return false;
    }

    //Sends yielding signal
    if (irtInit) {
      sendYield();

      sleep(IR_CYCLE*2);

      if (!updateYield()) {
        return false;
      }
    }

    //Closes gripper
    gripper->close();
    gripper->wait();

    //Sends done signal
    sendDone();

    //Updates tower height and cargo
    cargo = towerHeights[tower] - blockNum;
    towerHeights[tower] -= cargo;
  }
  
  return true;
}

//Unloads block(s) on top of tower
bool TowerRobot::unload(int tower) {
  if (cargo > 0) {
    //Moves one block above top of tower
    setTurretTarget(tower);
    setSlideTarget(towerHeights[tower]);

    if (!moveToBlock(tower, slideTarget)) {
      return false;
    }

    //Opens gripper
    gripper->open();
    gripper->wait();

    //Sends done signal
    sendDone();

    //Updates tower height and cargo
    towerHeights[tower] += cargo;
    cargo = 0;
  }

  return true;
}

//Gets robot cargo
int TowerRobot::getCargo() {
  return cargo;
}

//Sets whether target positions are tracked
void TowerRobot::setTurretTracking(bool active) {
  turretTracking = active;
}
void TowerRobot::setSlideTracking(bool active) {
  slideTracking = active;
}

//Sets tracking parameters
void TowerRobot::setTurretTarget(int target) {
  if (turretTracking) {
    turretTarget = target;
  }
}
void TowerRobot::setSlideTarget(int target) {
  if (slideTracking) {
    slideTarget = target;
  }
}

//Scans color of particular block
int TowerRobot::scanBlock(int tower, int blockNum) {
  if (colorInit) {
    //Opens gripper to clear towers
    gripper->open();

    //Moves to tower clockwise from target to align color sensor with target
    setTurretTarget(turret->nextTower(tower, -1));
    setSlideTarget(blockNum);

    if (!moveToBlock(turret->nextTower(tower, -1), blockNum + sensorMargin)) {
      return -2;
    }

    //Gets color of block
    if (irtInit) {
      irt->waitChannel(2, COLOR_CYCLE);
    }

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
        //Starts synchronization
        irt->resetChannels();

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

//Begins yielding
void TowerRobot::beginYield() {
  if (irtInit) {
    //Activates pending mode
    if (yieldMode == DORMANT) {
      yieldMode = PENDING;
    }

    //Updates turret angle
    turretAngle = Utils::modulo(turret->currentPosition(), 90.0);

    //Updates slide position
    double newPos = Utils::modulo(slide->currentPosition(), 1.0);
  }
}

//Ends yielding
void TowerRobot::endYield() {
  if (irtInit) {
    //Activates dormant mode
    yieldMode = DORMANT;
  }
}

//Sends yielding signal
void TowerRobot::sendYield() {
  if (irtInit) {
    irt->waitChannel(2, IR_CYCLE);

    //Next tower
    unsigned int nextTower = turret->nextTowerTo(turretTarget);

    //Target indicator
    unsigned int toTarget = (int) (nextTower == turretTarget);

    //Chooses loading or unloading command
    if (cargo == 0) {
      //Sends load data with target indicator and next tower
      irt->send(MASTER_ADDRESS, LOAD, toTarget*4 + nextTower);
    } else {
      //Unload data with clear height and next tower
      unsigned int data = (slideTarget + cargo)*4 + nextTower;

      //Sets command based on target indicator
      unsigned int command;
      if (toTarget) {
        command = UNLOAD_TARGET;
      } else {
        command = UNLOAD_TRAVEL;
      }

      //Sends data
      irt->send(MASTER_ADDRESS, command, data);
    }
    
    irt->waitSend();
  }
}

//Sends done signal to previous tower
void TowerRobot::sendDone() {
  if (irtInit) {
    //Gets previous tower
    int prevTower = turret->prevTowerTo(turret->targetTower());

    if (prevTower != turretTarget) {
      //Sends previous tower if not the same as target tower
      irt->waitChannel(2, IR_CYCLE);
      irt->send(MASTER_ADDRESS, DONE, prevTower);
      irt->waitSend();
    }
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
      //Sends done signal to previous tower
      sendDone();

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
        irt->nextChannel(IR_CYCLE);

        //Gets next tower
        int nextTower = turret->nextTowerTo(turretTarget);

        //Whether robot is heading to target
        bool toTarget = (nextTower == turretTarget);

        //If next tower matches
        if (data % 4 == nextTower) {
          if (command == DONE) {
            //Resumes if blocked
            yieldMode = PENDING;
          } else if (yieldMode = PENDING) {
            //Whether other robot is loading
            bool otherLoading;

            //Whether other robot is going to target
            bool otherToTarget;

            //Sets values based on command
            if (command == LOAD) {
              otherLoading = true;

              //Gets target state based on indicator
              otherToTarget = (data / 4);
            } else {
              otherLoading = false;

              //Gets target state based on command
              otherToTarget = (command == UNLOAD_TARGET);
            }

            if ((toTarget || otherToTarget) || ((cargo > 0) && !otherLoading)) {
              //If any robot is going to target or both are unloading
              if ((cargo > 0) && otherLoading) {
                //If unloading, ensures other robot is blocked if it is loading
                sendYield();
              } else {
                //Activates blocking mode
                yieldMode = BLOCKED;

                //Complete block occurs if both are target state
                blocked = toTarget && otherToTarget;

                //If other robot is unloading
                if (!otherLoading) {
                  //If unloading, move to clear other robot
                  if (cargo > 0) {
                    slide->moveToClear(data / 4);
                    slide->wait();
                  }
                  
                  //Move to carry position to avoid interference
                  turret->moveToCarry(nextTower);
                  turret->wait();
                }
              }
            }
          }
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

//Finds height of tower
int TowerRobot::findHeight(int tower) {
  return findHeight(tower, new int[10]);
}
int TowerRobot::findHeight(int tower, int* bufferColors) {
  //Gets predicted tower height
  int currHeight = towerHeights[tower];
  int currBlock = currHeight;

  //Resets buffer color array
  for (int i = 0; i < 10; i++) {
    *(bufferColors + i) = -2;
  }
  
  //Finds actual tower height
  bool startedEmpty = false;
  while (currBlock >= 0) {
    int checkColor = scanBlock(tower, currBlock);

    //Checks whether the first check was empty
    if (currBlock == currHeight) {
      startedEmpty = (checkColor == EMPTY);
    }

    //Updates buffer array with color
    if (checkColor != EMPTY) {
      *(bufferColors + currBlock) = checkColor;
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

  //Returns height
  return towerHeights[tower];
}


//Scans tower for target blocks
int TowerRobot::scanTower(int tower, int color, bool* startedTarget, int* bufferColors) {
  //Loops through checked colors and then finds more colors if necessary
  int currBlock;
  for (currBlock = towerHeights[tower]; currBlock > 0;) {
    currBlock--;

    //If color is not read, reads it in
    int checkColor = *(bufferColors + currBlock);
    if (checkColor == -2) {
      checkColor = scanBlock(tower, currBlock);
    }

    //Checks whether the top of the tower was the target color
    if (currBlock == (towerHeights[tower] - 1)) {
      *startedTarget = (checkColor == color);
    }

    //Breaks if color changes away or to target color
    if (*startedTarget != (checkColor == color)) {
      //Moves to block before change
      currBlock++;
      break;
    }
  }

  //Returns wether tower started with target blocks
  return currBlock;
}