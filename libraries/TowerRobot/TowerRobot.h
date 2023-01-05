/*
  TowerRobot.h - Controls components of robot
  Created by Carson G. Ray, January 4 2022.
*/

#ifndef TowerRobot_h
#define TowerRobot_h

#include "Arduino.h"
#include "Slide.h"
#include "Turret.h"
#include "Gripper.h"

class TowerRobot {
	private:
		Slide slide;

        Turret turret;

        Gripper gripper;
	public:
    	TowerRobot();
        moveToBlock(int num, int loadState);

        load();
        unload();

        rotateToScan();
        rotateToTower();
};

#endif