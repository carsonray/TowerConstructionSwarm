/*
  TowerRobot.h - Controls components of robot
  Created by Carson G. Ray, January 4 2022.
*/

#ifndef TowerRobot_h
#define TowerRobot_h

#include "Arduino.h"
#include "ScaledStepper.h"
#include "Servo.h"

class TowerRobot {
	public:
    	TowerRobot();

		class Slide {
			private:
				//Steps per block
				double stepsPerBlock;

				//Rack and pinion drive stepper
				ScaledStepper stepper;

				//Lower limit switch
				Button limit;

				//Homing speed
				double homeSpeed = -0.5;

				//Homing position
				double homePos = 0;

				//Upper limit
				double upperLimit;

				//Default acceleration
				double defAccel = 0.5;

				//Default max speed
				double defMax = 2;

				double convertToBlock(double raw);
				double convertToRaw(double block);
			public:
				Slide(double stepsPerBlock, double upperLimit, ScaledStepper stepper, Button limit);

				double distanceToGo();
				void wait();

				double currentPosition();
				double targetPosition();

				bool checkLimits();
				bool checkLowerLimit();
				bool checkUpperLimit();

				bool run();
				void stop(bool brake);

				void home();
				void home(double homePos);

				void moveToBlock(double blockPos);
				void moveToBlock(double blockPos, double accel, double max);
				
				void moveByBlock(double blockRel);
				void moveByBlock(double blockRel, double accel, double max);
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