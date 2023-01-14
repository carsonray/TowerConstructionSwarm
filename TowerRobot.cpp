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

  //Moves to correct tower and turret
  turret.moveToTower(tower);
  slide.moveToBlock(blockNum);
  turret.wait();
  slide.wait();

  //Closes gripper and clears blocks below it
  gripper.close();
  gripper.wait();
  slide.moveByBlock(clearMargin);
  slide.wait();

  //Updates tower height
  towerHeights[tower] = blockNum;
}

//Unloads block(s) on top of tower
void TowerRobot::unload(int tower) {
  
}