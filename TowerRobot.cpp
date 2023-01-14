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

//Loads block(s) from position on tower
void TowerRobot::load(int tower) {
  //Loads from top of tower as default
  load(tower, towerHeights[tower] - 1);
}
void TowerRobot::load(int tower, int blockNum) {
  //Opens gripper
  gripper.open();

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
  moveToBlock(tower, towerHeights[tower]);

  //Opens gripper
  gripper.open();
  gripper.wait();

  //Updates tower height and cargo
  towerHeights[tower] += cargo;
  cargo = 0;
}