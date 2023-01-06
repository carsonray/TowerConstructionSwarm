/*
  TowerRobot.h - Controls components of robot
  Created by Carson G. Ray, January 4 2022.
*/

#ifndef TowerRobot_h
#define TowerRobot_h

#include "Arduino.h"
#include "AccelStepper.h"
#include "Turret.h"
#include "Gripper.h"

class TowerRobot {
	public:
    	TowerRobot();
		/*
      	void moveToBlock(int num, int loadState);

      	void load();
      	void unload();

      	void rotateToScan();
      	void rotateToTower();
		*/

		class Slide {
			private:
				AccelStepper stepper;
				int stepsPerBlock;
				int stepPin;
				int dirPin;
				int modePins[3];
			public:
				Slide(int spb, int step, int dir, int mode1, int mode2, int mode3);

				void moveToBlock(int num);
				void update();
				int distanceToGo();
		};

	private:
		//Slide slide;

        Turret turret;

        Gripper gripper;
};

#endif