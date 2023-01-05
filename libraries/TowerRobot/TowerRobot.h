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
      	void moveToBlock(int num, int loadState);

      	void load();
      	void unload();

      	void rotateToScan();
      	void rotateToTower();
};

#endif