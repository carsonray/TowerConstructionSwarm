/*
  Gripper.cpp - Controls parallel lever gripper
  Created by Carson G. Ray, January 4 2022.
*/

#include <Arduino.h>
#include <Servo.h>
#include "TowerRobot.h"

TowerRobot::Gripper::Gripper(int gripPin) {
  //Attaches servo
	this->gripPin = gripPin;
}

//Attaches servo to pin
void TowerRobot::Gripper::begin() {
  servo.attach(gripPin);
}

//Opens gripper
void TowerRobot::Gripper::open() {
  setOpen(true);
}

//Closes gripper
void TowerRobot::Gripper::close() {
  setOpen(false);
}

//Whether gripper is open
bool TowerRobot::Gripper::isOpen() {
  return openState;
}

//Sets gripper open state
void TowerRobot::Gripper::setOpen(bool openState) {
  this->openState = openState;

  if (openState) {
    servo.write(gripPos[0]);
  } else {
    servo.write(gripPos[1]);
  }

  //Updates action timer
  lastAction = millis();
  waitTime = 1250;
}

//Waits for action to complete
void TowerRobot::Gripper::wait() {
  while ((millis() - lastAction) < waitTime) {
    
  }
}

//Toggles gripper open state
bool TowerRobot::Gripper::toggle() {
  setOpen(!openState);
  return openState;
}