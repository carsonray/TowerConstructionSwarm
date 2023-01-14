/*
  TowerRobot.h - Controls components of robot
  Created by Carson G. Ray, January 4 2022.
*/

#ifndef TowerRobot_h
#define TowerRobot_h

#include "Arduino.h"
#include "ScaledStepper.h"
#include "Servo.h"
#include "Utils.h"

class TowerRobot {
	public:
    	TowerRobot(Slide slide, Turret turret, Gripper gripper);

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
				//Steps per degree
				double stepsPerDegree;

				//Ring drive stepper
				ScaledStepper stepper;

				//Default acceleration
				double defAccel = 2;

				//Default max speed
				double defMax = 45;

				//Tower positions
				double[4] towerPos = {0, 90, 180, 270, 360};

				//Carry offset
				double carryOffset = 45;

				double convertToDegree(double raw);
				double convertToRaw(double degree);

				double localize(double globalAngle)
			public:
				Turret(double stepsPerDegree, ScaledStepper stepper);

				double distanceToGo();
				void wait();

				double currentPosition();
				double currentPosition(bool global);

				double targetPosition()
				double targetPosition(bool global);

				bool run();
				void stop(bool brake);

				void moveTo(bool global, double degree);
				void moveTo(bool global, double degree, double accel, double max);
				
				void moveBy(double relDegree);
				void moveBy(double relDegree, double accel, double max);

				void moveToTower(int tower);
				void moveToTower(int tower, double accel, double max);

				void moveToCarry(int tower);
				void moveToCarry(int tower, double accel, double max);
		};

		class Gripper {
			private:
				//Pin for servo signal
				int gripPin;

				//Servo object
				Servo servo;

				//Whether gripper is open
				bool openState;

				//Time of last action
				unsigned long lastAction = 0;

				//Wait time after action
				unsigned long waitTime = 0;

				//Positions
				int[2] gripPos = {40, 160}
			public:
				Gripper(int gripPin);

				void open();
				void close();

				bool isOpen();
				void setOpen(bool openState);

				void wait();

				void toggle();
		};

		void load(int tower);
		void load(int tower, int blockNum);

		void unload(int tower);

	private:
		Slide slide;
		Turret turret;
		Gripper gripper;

		//Block heights of each tower
		int[4] towerHeights = {0, 0, 0, 0};

		//Current number of block cargo
		int cargo = 0;

		//Margin to clear blocks after loading
		double clearMargin = 0.1;
};

#endif