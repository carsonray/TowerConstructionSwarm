/*
  Turret.cpp - Controls ring gear drive to rotate robot
  Created by Carson G. Ray, January 4 2022.
*/

#include "Arduino.h"
#include "ScaledStepper.h"
#include "TowerRobot.h"
#include "Utils.h"

TowerRobot::Turret::Turret(double stepsPerDegree, ScaledStepper* stepper) {
	this->stepsPerDegree = stepsPerDegree;
  this->stepper = stepper;
  stepper->enableModeSwitch();
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

int TowerRobot::Turret::getTowerPos() {
  return currTowerPos;
}

//Gets next tower traveling from start to end
int TowerRobot::Turret::nextTower(int start, int end) {
  //Gets shortest direction of travel
  int dir = Utils::sign(localize(towerPos[end] - towerPos[start]));

  //Increments tower position
  int newTower = start + dir;

  // Wraps around array positions
  if (newTower < 0) {
    newTower += 4;
  } else if (newTower >= 4) {
    newTower -= 4;
  }

  return newTower;
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
  currTowerPos = tower;
}

//Moves to carry position next to tower
void TowerRobot::Turret::moveToCarry(int tower) {
  moveToCarry(tower, defAccel, defMax);
}
void TowerRobot::Turret::moveToCarry(int tower, double accel, double max) {
  //Difference to target position
  double diff = localize(towerPos[tower] - currentPosition());

  //Corrects target position with carry offset
  double target = towerPos[tower] - (carryOffset * Utils::sign(diff));
  
  moveTo(false, target, accel, max);

  //Sets tower position
  currTowerPos = tower;
}