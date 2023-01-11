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
	slide = Slide();
}

TowerRobot::Slide() {
    
}