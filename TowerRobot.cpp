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

//Gets closest staggered position to target position
int TowerRobot::getStaggerPos(int blockPos) {
  //Gets current slide position
  int currPos = slide->targetBlock();

  //Gets modulo equivalent of robot address based on current position
  int staggered = currPos/staggerNum*staggerNum + irt->getAddress() % staggerNum;

  //If stagger position can fit closer to current and target positions
  int dist = (currPos + blockPos) - staggered*2;
  if (abs(dist) > staggerNum) {
    staggered += staggerNum*Utils::sign(dist);
  }

  //Ensures stagger position is greater than zero
  if (staggered < 0) {
    staggered += staggerNum;
  }

  return staggered;
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
    if (irtInit && (tower != turret->closestTower())) {
      //Moves slide to closest staggered level to avoid other robots
      slide->moveToBlock(getStaggerPos(blockNum));
      turret->moveToCarry(turret->nextTowerTo(tower));
      bool slideRun = true;
      bool turretRun = true;
      bool finalMove = false;
      while (slideRun || turretRun || (!finalMove)) {
        if (!updateYield()) {
          return false;
        }

        //Runs slide and turret
        slideRun = slide->run();
        turretRun = turret->run();

        //Moves turret to final position if slide is done
        if (!slideRun) {
          turret->moveToTower(tower);

          //Moves slide to final position if turret is close enough
          if ((tower == turret->closestTower()) && (!finalMove)) {
            slide->moveToBlock(blockNum);
            finalMove = true;
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
    
    //First checks current position if at tower
    int testPos;
    if (turret->atTower(turret->closestTower())) {
      testPos = turret->closestTower();
    } else {
      testPos = turret->nextTowerTo(tower);
    }

    //Loops through towers between current and target
    while (true) {
      //Gets height of test tower
      int clearHeight = towerHeights[testPos];

      //Uses current block position if higher
      if (slide->targetBlock() > clearHeight) {
        clearHeight = slide->targetBlock();
      }

      //Ensures unloading robots are staggered
      if (irtInit) {
        clearHeight = getStaggerPos(clearHeight);

        //Ensures staggered height is greater than tower height
        while (clearHeight < towerHeights[testPos]) {
          clearHeight += irt->getChannels();
        }
      }
      
      //If current position will not clear height or tower
      if ((slide->targetBlock() != clearHeight) || (slide->currentPosition() - (towerHeights[testPos] + slide->getClearMargin()) < -slide->getStepError())) {
        //Moves to position and clears tower if necessary
        if (clearHeight == towerHeights[testPos]) {
          slide->moveToClear(clearHeight);
        } else {
          slide->moveToBlock(clearHeight);
        }

        //Moves to carry position next to tower if not already at tower
        if (!turret->atTower(testPos)) {
          turret->moveToCarry(testPos);
        }

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

      sleep(IR_CYCLE);

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

    //Sends yielding signal
    if (irtInit) {
      sendYield();

      sleep(IR_CYCLE);

      if (!updateYield()) {
        return false;
      }
    }

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

//Sets tracking parameters
void TowerRobot::setTurretTarget(int target) {
  turretTarget = target;
}
void TowerRobot::setSlideTarget(int target) {
  slideTarget = target;
}

//Scans color of particular block
int TowerRobot::scanBlock(int tower, int blockNum) {
  if (colorInit) {
    //Opens gripper to clear towers
    gripper->open();

    //Moves to tower clockwise from target to align color sensor with target
    setTurretTarget(-1);
    setTurretTarget(-1);
    while (!moveToBlock(turret->nextTower(tower, -1), blockNum + sensorMargin)) {
      
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
      irt->resume();

      if (command == DONE) {
        //Starts synchronization
        irt->resetChannels();

        //Sends with minimal delay
        sleep(60);
        int hold = irt->getChannels();
        irt->setChannels(1);
        irt->send(MASTER_ADDRESS, DONE, data);
        irt->waitSend();

        irt->setChannels(hold);
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
  if (irtInit && (yieldMode == PENDING) && (turretTarget >= 0)) {
    //Next tower
    unsigned int nextTower = turret->nextTower();

    //Target indicator
    unsigned int toTarget = (int) (nextTower == turretTarget);

    //Chooses loading or unloading command
    if (cargo == 0) {
      //Sends load data with target indicator and next tower
      irt->send(MASTER_ADDRESS, LOAD, toTarget*4 + nextTower);
    } else {
      //Unload data with clear height and next tower
      unsigned int data = (slide->targetBlock() + cargo)*4 + nextTower;

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
    
    //Waits until sent unless something is received
    irt->setInterrupt();
    irt->waitSend();
  }
}

//Sends done signal to previous tower
void TowerRobot::sendDone() {
  if (irtInit && (yieldMode == PENDING)) {
    //Sends done at previous tower
    irt->send(MASTER_ADDRESS, DONE, turret->targetTower());
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
      //Sends yield signal for next tower
      sendYield();
    }

    //Updates turret angle
    turretAngle = newAngle;

    //Loops until not blocked
    while (true) {
      //Updates signals
      irt->update();
      if (irt->receive(&command, &data)) {
        irt->resume();

        //Gets next tower
        int nextTower = turret->nextTower();

        //Whether robot is heading to target
        bool toTarget = (nextTower == turretTarget);

        //If next tower matches
        if (data % 4 == nextTower) {
          if (command == DONE) {
            //Unblocks if done sent at tower
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

            if ((cargo > 0) && !otherLoading) {
              //If both robots are unloading, checks for higher robot
              if (slide->targetBlock() + cargo >= data / 4) {
                //Blocks if targets match
                if (toTarget && otherToTarget) {
                  yieldMode = BLOCKED;
                  blocked = true;
                }

                //Moves to clear other robot
                if (slide->targetBlock() <= data / 4) {
                  int clearHeight = getStaggerPos(data / 4);
                  while(clearHeight < data / 4) {
                    clearHeight += irt->getChannels();
                  }
                  slide->moveToClear(clearHeight);
                  slide->wait();
                }
              } else {
                //Blocks other robot if it is higher
                sendYield();
              }
            } else if (toTarget && otherToTarget) {
              if ((cargo > 0) && otherLoading) {
                //If unloading, block loading robots
                sendYield();
              } else {
                //Otherwise pauses until other robot is done
                yieldMode = BLOCKED;
                blocked = true;

                //If loading and other is unloading, move to carry position to avoid interference
                if ((cargo == 0) && !otherLoading) {
                  turret->moveToCarry(turret->nextTower());
                  turret->wait();
                }
              }
            }
          }
        } else if (yieldMode == BLOCKED) {
          //Unblocks if yield sent at another tower
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
      irt->resume();

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