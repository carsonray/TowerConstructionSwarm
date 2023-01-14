/*
  Gripper.cpp - Controls parallel lever gripper
  Created by Carson G. Ray, January 4 2022.
*/

#include "Arduino.h"
#include "Servo.h"
#include "TowerRobot.h"

TowerRobot::Gripper::Gripper(int gripPin) {
  //Attaches servo
	this->gripPin = gripPin;
  servo.attach(gripPin);

  //Initializes gripper open
  open();
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
}

//Toggles gripper open state
bool TowerRobot::Gripper::toggle() {
  setOpen(!openState);
  return openState;
}