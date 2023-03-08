/*
  Turret.cpp - Controls ring gear drive to rotate robot
  Created by Carson G. Ray, January 4 2022.
*/

#include <Arduino.h>
#include "ScaledStepper.h"
#include "TowerRobot.h"
#include "Utils.h"

TowerRobot::Turret::Turret(double stepsPerDegree, ScaledStepper* stepper) {
	this->stepsPerDegree = stepsPerDegree;
  this->stepper = stepper;
  stepper->setStepMode(8);
  stepper->setMaxSpeed(convertToRaw(defMax));
}

//Converts raw steps to degrees
double TowerRobot::Turret::convertToDegree(double raw) {
  return raw/stepsPerDegree;
}

//Converts degrees to raw steps
double TowerRobot::Turret::convertToRaw(double degree) {
  return degree*stepsPerDegree;
}

//Gets local angle from global angle
double TowerRobot::Turret::localize(double globalAngle) {
  return globalAngle - round(globalAngle/360)*360;
}

//Gets distance to target position
double TowerRobot::Turret::distanceToGo() {
  return convertToDegree(stepper->distanceToGo());
}

//Waits until Turret is not moving
void TowerRobot::Turret::wait() {
  //Runs Turret while waiting to stop
  while (run()) {
    
  }
}

//Homes turret at zero position
void TowerRobot::Turret::home() {
  //Zeros stepper position
  stepper->setCurrentPosition(0);
}

//Returns current block position
double TowerRobot::Turret::currentPosition() {
  return currentPosition(true);
}
double TowerRobot::Turret::currentPosition(bool global) {
  double rawPos = convertToDegree(stepper->currentPosition());

  if (!global) {
    rawPos = localize(rawPos);
  }
  
  return rawPos;
}

//Returns block target position
double TowerRobot::Turret::targetPosition() {
  return targetPosition(true);
}
double TowerRobot::Turret::targetPosition(bool global) {
  double rawPos = convertToDegree(stepper->targetPosition());

  if (!global) {
    rawPos = localize(rawPos);
  }
  
  return rawPos;
}

//Gets position of tower
double TowerRobot::Turret::getTowerPos(int tower) {
  return towerPos[tower];
}

//Gets current tower position
int TowerRobot::Turret::targetTower() {
  return targetTowerPos;
}

double TowerRobot::Turret::getStepError() {
  return convertToDegree(0.5/stepper->getStepMode());
}

//Gets closest tower position
int TowerRobot::Turret::closestTower() {
  double minDist = 0;
  int minPos = 0;
  for (int i = 0; i < numPos(); i++) {
    double dist = abs(localize(towerPos[i] - currentPosition()));

    if ((dist < minDist) || (i == 0)) {
      minDist = dist;
      minPos = i;
    }
  }
  
  return minPos;
}

//Gets number of tower positions
int TowerRobot::Turret::numPos() {
  return sizeof(towerPos)/sizeof(towerPos[0]);
}

//Gets next tower in direction
int TowerRobot::Turret::nextTower(int change) {
  return nextTower(targetTowerPos, change);
}
int TowerRobot::Turret::nextTower(int curr, int change) {
  //Increments tower position
  return Utils::modulo(curr + change, numPos());
}


//Gets next tower traveling to target
int TowerRobot::Turret::nextTowerTo(int target) {
  return nextTowerTo(targetTowerPos, target);
}
int TowerRobot::Turret::nextTowerTo(int curr, int target) {
  //Gets next tower in shortest direction of travel
  return nextTower(curr, Utils::sign(localize(towerPos[target] - towerPos[targetTowerPos])));
}

//Runs Turret step
bool TowerRobot::Turret::run() {
  return stepper->run();
}

//Stops Turret
void TowerRobot::Turret::stop(bool brake) {
  if (brake) {
    //Soft stop
    stepper->stop();
  } else {
    //Hard stop
    stepper->setSpeed(0);
  }
}

//Moves to block position
void TowerRobot::Turret::moveTo(bool global, double degree) {
  moveTo(global, degree, defAccel, defMax);
}
void TowerRobot::Turret::moveTo(bool global, double degree, double accel, double max) {
  //If local target, add to local position
  if (!global) {
    degree = currentPosition() + (localize(degree - currentPosition()));
  }

  //Sets stepper settings
  stepper->setAcceleration(convertToRaw(accel));
  stepper->setMaxSpeed(convertToRaw(max));
  stepper->moveTo(convertToRaw(degree));
}

//Moves relatively by blocks
void TowerRobot::Turret::moveBy(double relDegree) {
  moveBy(relDegree, defAccel, defMax);
}
void TowerRobot::Turret::moveBy(double relDegree, double accel, double max) {
  moveTo(true, currentPosition() + relDegree, accel, max);
}

//Moves to tower position
void TowerRobot::Turret::moveToTower(int tower) {
  moveToTower(tower, defAccel, defMax);
}
void TowerRobot::Turret::moveToTower(int tower, double accel, double max) {
  moveTo(false, towerPos[tower], accel, max);

  //Sets tower position
  targetTowerPos = tower;
}

//Moves to carry position next to tower
void TowerRobot::Turret::moveToCarry(int tower) {
  moveToCarry(tower, defAccel, defMax);
}
void TowerRobot::Turret::moveToCarry(int tower, double accel, double max) {
  //Direction to target position
  double dir = Utils::sign(localize(towerPos[tower] - currentPosition()));

  //Ensures dir is not zero
  if (dir == 0) {
    dir = 1;
  }

  //Corrects target position with carry offset
  double target = towerPos[tower] - (carryOffset * dir);
  
  moveTo(false, target, accel, max);

  //Sets tower position
  targetTowerPos = tower;
}