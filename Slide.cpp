/*
  Slide.cpp - Controls rack and pinion drive for vertical movement
  Created by Carson G. Ray, January 4 2022.
*/

#include "Arduino.h"
#include "ScaledStepper.h"
#include "Button.h"
#include "TowerRobot.h"

TowerRobot::Slide::Slide(double stepsPerBlock, double upperLimit, ScaledStepper* stepper, Button* limit) {
	this->stepsPerBlock = stepsPerBlock;
  this->upperLimit = upperLimit;
  this->stepper = stepper;
  stepper->enableModeSwitch();
  stepper->setMaxSpeed(convertToRaw(defMax));
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
  return convertToBlock(stepper->distanceToGo());
}

//Waits until slide is not moving
void TowerRobot::Slide::wait() {
  //Runs slide while waiting to stop
  while(run()) {

  }
}

//Returns current block position
double TowerRobot::Slide::currentPosition() {
  return convertToBlock(stepper->currentPosition());
}

//Returns block target position
double TowerRobot::Slide::targetPosition() {
  return convertToBlock(stepper->targetPosition());
}

//Whether slide will run through limits
bool TowerRobot::Slide::checkLimits() {
  //Gets current stepper speed
  double currSpeed = convertToBlock(stepper->speed());

  //Checks limits
  bool lower = checkLowerLimit();
  bool upper = checkUpperLimit();

  //If lower limit is tripped going down or upper limit is tripped going up
  return (lower && (currSpeed < 0)) || (upper && (currSpeed > 0));
}

//Whether slide is at physical limit switch
bool TowerRobot::Slide::checkLowerLimit() {
  bool pressed = false;
  //Whether limit switch changes state to pressed
  if (limit->changeTo(true)) {
    pressed = true;
  }
  limit->update();
  return pressed;
}

//Whether slide is at upper block limit
bool TowerRobot::Slide::checkUpperLimit() {
  return (currentPosition() >= upperLimit);
}

//Runs slide step
bool TowerRobot::Slide::run() {
  if (checkLimits()) {
    return false;
  } else {
    return stepper->run();
  }
}

//Stops slide
void TowerRobot::Slide::stop(bool brake) {
  if (brake) {
    //Soft stop
    stepper->stop();
  } else {
    //Hard stop
    stepper->setSpeed(0);
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
  stepper->setSpeed(convertToRaw(homeSpeed));

  while (!checkLimits()) {
    stepper->runSpeed();
  }

  //Homes when limit is reached
  stepper->setCurrentPosition(convertToRaw(homePos));
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
  stepper->setAcceleration(convertToRaw(accel));
  stepper->setMaxSpeed(convertToRaw(max));
  stepper->moveTo(convertToRaw(blockPos));
}

//Moves relatively by blocks
void TowerRobot::Slide::moveByBlock(double blockRel) {
  moveByBlock(blockRel, defAccel, defMax);
}
void TowerRobot::Slide::moveByBlock(double blockRel, double accel, double max) {
  moveToBlock(currentPosition() + blockRel, accel, max);
}