/*
  TowerRobot.cpp - Controls components of robot
  Created by Carson G. Ray, January 4 2022.
*/

#include "Arduino.h"
#include "AccelStepper.h"
#include "TowerRobot.h"

TowerRobot::TowerRobot(Slide* slide, Turret* turret, Gripper* gripper) {
	this->slide = slide;
  this->turret = turret;
  this->gripper = gripper;
}

//Sets tower heights
void TowerRobot::setTowerHeights(int tower1, int tower2, int tower3, int tower4) {
  towerHeights[0] = tower1;
  towerHeights[1] = tower2;
  towerHeights[2] = tower3;
  towerHeights[3] = tower4;
}

//Homes robot
void TowerRobot::home() {
  home(0);
}
void TowerRobot::home(double homePos) {
  gripper->begin();
  gripper->open();
  slide->home(homePos);
  turret->moveTo(false, 0);
}

void TowerRobot::waitSlideTurret() {
  while (slide->run() || turret->run()) {

  }
}

//Moves to tower and block position
void TowerRobot::moveToBlock(int tower) {
  //Moves to top of tower as default
  moveToBlock(tower, towerHeights[tower] - 1);
}
void TowerRobot::moveToBlock(int tower, int blockNum) {
  if (cargo == 0) {
    //No cargo

    //Opens gripper to clear towers
    gripper->open();

    //Moves to correct position
    slide->moveToBlock(blockNum);
    turret->moveToTower(tower);
    waitSlideTurret();
  } else {
    //Loops through towers between current and target
    for (int testPos = turret->getTowerPos(); testPos != tower; testPos = turret->nextTower(testPos, tower)) {
      //If current position will not clear tower
      if (slide->currentPosition() < (towerHeights[testPos] + clearMargin)) {
        //Moves to height of obstructing tower + margin
        slide->moveToBlock(towerHeights[testPos] + clearMargin);

        //Moves to carry position next to tower
        if (testPos != turret->getTowerPos())
          turret->moveToCarry(testPos);

        waitSlideTurret();
      }
    }

    //Moves final step to tower and block position
    turret->moveToTower(tower);
    turret->wait();
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
    //Moves to top of tower
    moveToBlock(tower);

    //Opens gripper
    gripper->open();
    gripper->wait();

    //Updates tower height and cargo
    towerHeights[tower] += cargo;
    cargo = 0;
  }
}