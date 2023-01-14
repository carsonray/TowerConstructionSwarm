/*
  TowerRobot.h - Controls components of robot
  Created by Carson G. Ray, January 4 2022.
*/

#ifndef TowerRobot_h
#define TowerRobot_h

#include "Arduino.h"
#include "ScaledStepper.h"
#include "Servo.h"
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

		class Turret {
			private:

			public:
				Turret();
		};

		class Gripper {
			private:
				//Pin for servo signal
				int gripPin;

				//Servo object
				Servo servo;

				//Whether gripper is open
				bool openState;

				//Positions
				int[2] gripPos = {40, 160}
			public:
				Gripper(int gripPin);

				void open();
				void close();

				bool isOpen();
				void setOpen(bool openState);

				void toggle();
		};

	private:
		Slide slide;
		Turret turret;
		Gripper gripper;
};

#endif