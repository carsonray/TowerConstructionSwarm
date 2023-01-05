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

TowerRobot::moveToBlock(int num) {
    moveToBlock(num, 0);
}
TowerRobot::moveToBlock(int num, int loadState) {
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

TowerRobot::load() {
    turret.rotateToTower();
    gripper.close();
}

TowerRobot::unload() {
    turret.rotateToTower()
    gripper.open();
}

TowerRobot::rotateToScan() {
    turret.rotateToScan();
}

TowerRobot::rotateToTower() {
    turret.rotateToTower();
}