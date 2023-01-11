/*
  TowerRobot.h - Controls components of robot
  Created by Carson G. Ray, January 4 2022.
*/

#ifndef TowerRobot_h
#define TowerRobot_h

#include "Arduino.h"
#include "ScaledStepper.h"
#include "Turret.h"
#include "Gripper.h"

class TowerRobot {
	public:
    	TowerRobot();

		class Slide {
			private:
				
			public:
				Slide();
		};

	private:
		Slide slide;
};

#endif