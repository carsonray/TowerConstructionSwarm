/*
  Slide.cpp - Controls rack and pinion drive for vertical movement
  Created by Carson G. Ray, January 4 2022.
*/

#include "Arduino.h"
#include "ScaledStepper.h"
#include "Button.h"
#include "TowerRobot.h"

TowerRobot::Slide::Slide(double stepsPerBlock, double upperLimit, ScaledStepper stepper, Button limit) {
	this->stepsPerBlock = stepsPerBlock;
  this->upperLimit = upperLimit;
  this->stepper = stepper;
  stepper.enableModeSwitch();
  this->limit = limit;
}

//Converts raw steps to blocks
double TowerRobot::Slide::convertToBlock(double raw) {
  return raw/stepsPerBlock;
}

//Converts blocks to raw steps
double TowerRobot::Slide::convertToRaw(double block) {
  return block*stepsPerBlock;
}

//Gets distance to target position
double TowerRobot::Slide::distanceToGo() {
  return convertToBlock(stepper.distanceToGo());
}

//Waits until slide is not moving
void TowerRobot::Slide::wait() {
  //Runs slide while waiting to stop
  while (run()) {
    
  }
}

//Returns current block position
double TowerRobot::Slide::currentPosition() {
  return convertToBlock(stepper.currentPosition());
}

//Returns block target position
double TowerRobot::Slide::targetPosition() {
  return convertToBlock(stepper.targetPosition());
}

//Whether slide is at a limit
bool TowerRobot::Slide::checkLimits() {
  return (checkLowerLimit() || checkUpperLimit());
}

//Whether slide is at physical limit switch
bool TowerRobot::Slide::checkLowerLimit() {
  if (limit.curr()) {
    //Sets to home position
    stepper.setPosition(homePos);
    return true;
  } else {
    return false;
  }
}

//Whether slide is at upper block limit
bool TowerRobot::Slide::checkUpperLimit() {
  if (currentPosition() >= upperLimit) {
    //Moves to upper limit
    moveToBlock(upperLimit);
    return true;
  } else {
    return false;
  }
}

//Runs slide step
bool TowerRobot::Slide::run() {
  if (checkLimits()) {
    return false;
  } else {
    return stepper.run();
  }
}

//Stops slide
void TowerRobot::Slide::stop(bool brake) {
  if (brake) {
    //Soft stop
    stepper.stop();
  } else {
    //Hard stop
    stepper.setSpeed(0);
  }
}

//Homes slide with physical limit switch at desired zero position
void TowerRobot::Slide::home() {
  home(homePos);
}
void TowerRobot::Slide::home(double homePos) {
  //Sets home position
  this->homePos = homePos;

  //Uses constant slow speed
  stepper.setAcceleration(0);
  stepper.setSpeed(convertToRaw(homeSpeed));
}

//Moves to block position
void TowerRobot::Slide::moveToBlock(double blockPos) {
  moveToBlock(blockPos, defAccel, defMax);
}
void TowerRobot::Slide::moveToBlock(double blockPos, double accel, double max) {
  //Ensures blockPos is within range
  if (blockPos < homePos) {
    blockPos = homePos;
  } else if (blockPos > upperLimit) {
    blockPos = upperLimit;
  }

  //Sets stepper settings
  stepper.setAcceleration(accel);
  stepper.setMaxSpeed(max);
  stepper.moveTo(convertToRaw(blockPos));
}

//Moves relatively by blocks
void TowerRobot::Slide::moveByBlock(double blockRel) {
  moveByBlock(blockRel, defAccel, defMax);
}
void TowerRobot::Slide::moveByBlock(double blockRel, double accel, double max) {
  moveToBlock(currentPosition() + blockRel, accel, max);
}