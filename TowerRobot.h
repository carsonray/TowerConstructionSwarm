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
#include "Button.h"
#include <Wire.h>
#include "Adafruit_TCS34725.h"

class TowerRobot {
	public:
		class Slide {
			private:
				//Steps per block
				double stepsPerBlock;

				//Rack and pinion drive stepper
				ScaledStepper* stepper;

				//Lower limit switch
				Button* limit;

				//Homing speed
				double homeSpeed = -0.2;

				//Homing position
				double homePos = 0;

				//Upper limit
				double upperLimit;

				//Default acceleration
				double defAccel = 1;

				//Default max speed
				double defMax = 3.5;

				double convertToBlock(double raw);
				double convertToRaw(double block);
			public:
				Slide(double stepsPerBlock, double upperLimit, ScaledStepper* stepper, Button* limit);

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

				//First direction that turret turns
				int firstDir = 0;

				//Correction for gear slop
				double gearCorrect = 5;

				//Ring drive stepper
				ScaledStepper* stepper;

				//Default acceleration
				double defAccel = 30;

				//Default max speed
				double defMax = 100;

				//Current tower position
				int currTowerPos = 0;

				//Tower positions
				double towerPos[4] = {0, 90, 180, 270};

				//Carry offset
				double carryOffset = 45;

				double convertToDegree(double raw);
				double convertToRaw(double degree);

				double localize(double globalAngle);
				int numPos();
			public:
				Turret(double stepsPerDegree, ScaledStepper* stepper);

				double distanceToGo();
				void wait();

				double currentPosition();
				double currentPosition(bool global);

				double targetPosition();
				double targetPosition(bool global);

				int getTowerPos();

				int nextTower(int dir);
				int nextTower(int curr, int dir);

				int nextTowerTo(int target);
				int nextTowerTo(int curr, int target);

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
				int gripPos[2] = {40, 185};
			public:
				Gripper(int gripPin);

				void begin();

				void open();
				void close();

				bool isOpen();
				void setOpen(bool openState);

				void wait();

				bool toggle();
		};

		class ColorSensor {
			private:
				//Color sensor object
				Adafruit_TCS34725 tcs;

				//Empty/block present threshold
				int emptyThres = 50;

				//Block color values (black, white, red, blue)
				int blockColors[4][3] = {
					{46, 75, 63},
					{1127, 2075, 1754},
					{176, 79, 66},
					{54, 184, 390}
				};

				int numColors();
			public:
				ColorSensor();
				bool begin();
				void getRaw(bool led, int* r, int*g, int*b, int* c);
				void getReflected(int* r, int*g, int*b, int* c);
				int getBlockColor();
		};

		TowerRobot(Slide* slide, Turret* turret, Gripper* gripper);
		TowerRobot(Slide* slide, Turret* turret, Gripper* gripper, ColorSensor* colorSensor);

		void setTowerHeights(int tower1, int tower2, int tower3, int tower4);

		void waitSlideTurret();

		void home();
		void home(double homePos);

		void moveToBlock(int tower);
		void moveToBlock(int tower, int blockNum);

		void load(int tower);
		void load(int tower, int blockNum);

		void unload(int tower);

		int scanBlock(int tower, int blockNum);
	private:
		Slide* slide;
		Turret* turret;
		Gripper* gripper;
		ColorSensor* colorSensor;

		//Whether color sensor is initialized
		bool colorInit = false;

		//Block heights of each tower
		int towerHeights[4] = {0, 0, 0, 0};

		//Current number of block cargo
		int cargo = 0;

		//Margin to clear blocks after loading
		double clearMargin = 0.2;
};

#endif