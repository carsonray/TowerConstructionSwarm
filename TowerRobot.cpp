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