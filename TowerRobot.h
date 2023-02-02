/*
  TowerRobot.h - Controls components of robot
  Created by Carson G. Ray, January 4 2022.
*/

#ifndef TowerRobot_h
#define TowerRobot_h

#include <Arduino.h>
#include <Servo.h>
#include <Wire.h>
#include <Adafruit_TCS34725.h>
#include "ScaledStepper.h"
#include "Utils.h"
#include "Button.h"

//Commands

namespace IRcommands {
	//Delay cycle for responses
	#define DELAY_CYCLE 60

	//Address accepted by all
	#define MASTER_ADDRESS 0x0

	//Controller address
	#define CONTROL_ADDRESS 0x1
	
	//Command
  	#define IR_STATUS 0x0

	//Data options
	#define IR_STATUS_POLL 0x0
  	#define IR_STATUS_READY 0x1
	
  	#define IR_CURRENT_TOWER 0x1
	#define IR_CURRENT_HEIGHT 0x2

	#define IR_TURRET 0xA
	#define IR_SLIDE 0xB
	#define IR_GRIPPER 0xC
}

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
				double homePos = -0.1;

				//Upper limit
				double upperLimit;

				//Default acceleration
				double defAccel = 1;

				//Default max speed
				double defMax = 2.5;

				//Margin to clear blocks after loading
				double clearMargin = 0.3;

				//Current block position
				int currBlockPos = 0;

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

				double getHomePos();
				int getBlockPos();
				double getClearMargin();

				void moveToBlock(double blockPos);
				void moveToBlock(double blockPos, double accel, double max);

				void moveToClear(int blockPos);
				void moveToClear(int blockPos, double accel, double max);
				
				void moveByBlock(double blockRel);
				void moveByBlock(double blockRel, double accel, double max);
		};

		class Turret {
			private:
				//Steps per degree
				double stepsPerDegree;

				//Correction for gear slop
				double gearCorrect = 6;

				//Whether position is corrected
				bool correcting = false;

				//Ring drive stepper
				ScaledStepper* stepper;

				//Default acceleration
				double defAccel = 30;

				//Default max speed
				double defMax = 80;

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

		class IRT {
			private:
				//Sending pin
				int sendPin;

				//Receiving pin
				int recvPin;

				//Unique address
				int address;

				//Address bitmap (address, command)
				int bitMap[2] = {4, 4};

				//Whether sending is active
				bool sendActive = true;

				//Current sending address
				unsigned int sendAddress = 0;
				
				//Current sending command
				unsigned int sendCommand = 0;

				//Time of last send
				unsigned long lastSend = 0;

				//Current repeating interval
				int currInterval = 0;
				
				//Minimum repeating signal interval
				int minInterval = 0;

				//Maximum repeating signal interval
				int maxInterval = 0;

				//Current signal repeats
				int currRepeats = 0;

				//Limit on number of signal repeats
				int sendRepeats = 0;

				//Whether receiving is active
				bool recvActive = true;

				//Whether recieved signal is availiable
				bool recvExists = false;

				//Wait time to avoid receiving interference
				int sheildTime = 15;

				//Current recieved address
				unsigned int recvAddress = 0;

				//Current recieved command
				unsigned int recvCommand = 0;

				//Current recieved data
				unsigned int recvData = 0;

				//Whether signals for other addresses are automatically relayed
				bool autoRelay = false;
				
				void unpack(unsigned int address, unsigned int command);
			public:
				IRT(int address, int sendPin, int recvPin);

				void begin();

				void setSendActive(bool active);

				void send(unsigned int address, unsigned int command);
				void send(unsigned int address, unsigned int command, unsigned int data);

				void setSendInterval(int interval);
				void setSendInterval(int minTime, int maxTime);

				void setSendRepeats(int repeats);
				void resetSendRepeat();

				bool isSending();
				void waitSend();

				void setReceiveActive(bool active);

				bool receive(unsigned int*command, unsigned int*data);

				void waitReceive();
				void waitReceive(int timeout);

				void setAutoRelay(bool active);

				void update();

				void synchronize(int num, int interval);
		};

		TowerRobot(Slide* slide, Turret* turret, Gripper* gripper);
		TowerRobot(Slide* slide, Turret* turret, Gripper* gripper, ColorSensor* colorSensor);
		TowerRobot(Slide* slide, Turret* turret, Gripper* gripper, ColorSensor* colorSensor, IRT* irt);

		void setTowerHeights(int tower1, int tower2, int tower3, int tower4);
		int getTowerHeight(int tower);

		void waitSlideTurret();

		void home();
		void home(double homePos);

		void moveToBlock(int tower);
		void moveToBlock(int tower, int blockNum);

		void load(int tower);
		void load(int tower, int blockNum);

		void unload(int tower);

		int scanBlock(int tower, int blockNum);
		void synchronize();
	private:
		Slide* slide;
		Turret* turret;
		Gripper* gripper;
		ColorSensor* colorSensor;
		IRT* irt;

		//Whether color sensor is initialized
		bool colorInit = false;

		//Whether infrared tranciever is initialized
		bool irtInit = false;

		//Block heights of each tower
		int towerHeights[4] = {0, 0, 0, 0};

		//Current number of block cargo
		int cargo = 0;

		//Margin for color sensor to read block
		double sensorMargin = 0.15;
};

#endif