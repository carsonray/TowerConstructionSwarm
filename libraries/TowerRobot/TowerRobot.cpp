/*
  TowerRobot.cpp - Controls components of robot
  Created by Carson G. Ray, January 4 2022.
*/

#include "Arduino.h"
#include "TowerRobot.h"
#include "Slide.h"
#include "Turret.h"
#include "Gripper.h"

TowerRobot::TowerRobot() {
	slide = Slide();
    turret = Turret();
    gripper = Gripper();
}

void TowerRobot::moveToBlock(int num) {
    moveToBlock(num, 0);
}
void TowerRobot::moveToBlock(int num, int loadState) {
    slide.moveToBlock(num);
    switch (loadState) {
        case 1:
            load();
            break;
        case 2:
            unload();
            break;
        default:
            break;
    }
}

void TowerRobot::load() {
    turret.rotateToTower();
    gripper.close();
}

void TowerRobot::unload() {
    turret.rotateToTower()
    gripper.open();
}

void TowerRobot::rotateToScan() {
    turret.rotateToScan();
}

void TowerRobot::rotateToTower() {
    turret.rotateToTower();
}