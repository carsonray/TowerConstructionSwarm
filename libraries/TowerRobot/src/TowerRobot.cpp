/*
  TowerRobot.cpp - Controls components of robot
  Created by Carson G. Ray, January 4 2022.
*/

#include "Arduino.h"
#include "AccelStepper.h"
#include "TowerRobot.h"
#include "Turret.h"
#include "Gripper.h"

TowerRobot::TowerRobot() {
	//slide = Slide(222, 12, 13, 9, 10, 11);
    turret = Turret();
    gripper = Gripper();
}
/*
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
}*/

TowerRobot::Slide::Slide(int spb, int step, int dir, int mode1, int mode2, int mode3) {
	this->stepsPerBlock = spb;
	this->stepPin = step;
	this->dirPin = dir;
	this->modePins[0] = mode1;
	this->modePins[1] = mode2;
	this->modePins[2] = mode3;

	stepper = AccelStepper(1, stepPin, dirPin);

	for (int modePin: this->modePins) {
		pinMode(modePin, OUTPUT);
	}

	//digitalWrite(this->modePins[1], HIGH);
}

void TowerRobot::Slide::moveToBlock(int num) {
	stepper.setMaxSpeed(1000);
  	stepper.setAcceleration(50);
  	stepper.setSpeed(200);
  	stepper.moveTo(num*stepsPerBlock);
}
int TowerRobot::Slide::distanceToGo() {
    return stepper.distanceToGo();
}
void TowerRobot::Slide::update() {
    stepper.run();
}