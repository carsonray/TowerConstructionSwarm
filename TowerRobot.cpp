/*
  TowerRobot.cpp - Controls components of robot
  Created by Carson G. Ray, January 4 2022.
*/

#include "Arduino.h"
#include "AccelStepper.h"
#include "TowerRobot.h"

TowerRobot::TowerRobot(Slide slide, Turret turret, Gripper gripper) {
	this->slide = slide;
  this->turret = turret;
  this->gripper = gripper;
}

//Waits on all busy modules
void wait() {
  slide.wait();
  turret.wait();
  gripper.wait();
}

//Homes robot
void TowerRobot::home() {
  gripper.open();
  slide.home();
  turret.moveTo(false, 0);
}

//Moves to tower and block position
void TowerRobot::moveToBlock(int tower) {
  //Moves to top of tower as default
  moveToBlock(tower, towerHeights[tower] - 1)
}
void TowerRobot::moveToBlock(int tower, int blockNum) {
  if (cargo == 0) {
    //No cargo

    //Opens gripper to clear towers
    gripper.open();

    //Moves to correct position
    slide.moveToBlock(blockNum);
    turret.moveToTower(tower);
    slide.wait();
    turret.wait();
  } else {
    //Clears current tower
    if (slide.currentPosition() <= towerHeights[turret.currTowerPos()]) {
      slide.moveByBlock(clearMargin);
    }
    
    //Loops through towers between current and target
    for (int testPos = turret.currTowerPos(); testPos <= tower; testPos = turret.nextTower(testPos, tower)) {
      if (slide.currentPosition() > towerHeights[testPos]) {
        //If current position will not clear tower

        //Moves to height of obstructing tower + margin
        slide.moveToBlock(towerHeights[turret.currTowerPos()] + clearMargin);

        //Moves to carry position next to tower
        turret.moveToCarry(turret.currTowerPos());

        slide.wait();
        turret.wait();
      }
    }

    //Moves final step to tower and block position
    turret.moveToTower(tower);
    turret.wait();

    slide.moveToBlock(blockNum);
    slide.wait();
  }
}

//Loads block(s) from position on tower
void TowerRobot::load(int tower) {
  //Loads from top of tower as default
  load(tower, towerHeights[tower] - 1);
}
void TowerRobot::load(int tower, int blockNum) {
  //Moves to correct tower and block
  moveToBlock(tower, blockNum);

  //Closes gripper
  gripper.close();
  gripper.wait();

  //Updates tower height and cargo
  cargo = towerHeights[tower] - blockNum;
  towerHeights[tower] -= cargo;
}

//Unloads block(s) on top of tower
void TowerRobot::unload(int tower) {
  //Moves to top of tower
  moveToBlock(tower);

  //Opens gripper
  gripper.open();
  gripper.wait();

  //Updates tower height and cargo
  towerHeights[tower] += cargo;
  cargo = 0;
}