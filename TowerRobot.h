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

namespace BlockColors {
	#define EMPTY -1
	#define BLACK 0
	#define WHITE 1
	#define RED 2
	#define BLUE 3
}

namespace IRcommands {
	//Delay cycle for responses
	#define IR_CYCLE 60

	//Delay cycle for color sensor readings
	#define COLOR_CYCLE 300

	//Address accepted by all
	#define MASTER_ADDRESS 0x0

	//Controller address
	#define CONTROL_ADDRESS 0x1

	//Done signal
	#define DONE 0x0
	
	//Yielding updates                                                                                                                                   
  	#define LOAD 0x1
	#define UNLOAD_TRAVEL 0x2
	#define UNLOAD_TARGET 0x3

	//Remote control commands
	#define SLIDE 0xA
	#define TURRET 0xB
	#define CARRY 0xC
	#define GRIPPER 0xD                                                                                                                            
}

namespace YieldModes {
	#define DORMANT 0
	#define PENDING 1
	#define BLOCKED 2
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
				double defAccel = 2;

				//Default max speed
				double defMax = 2;

				//Margin to clear blocks after loading
				double clearMargin = 0.3;

				//Current block position
				int targetBlockPos = 0;

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
				int targetBlock();
				double getClearMargin();
				double getStepError();

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

				//Ring drive stepper
				ScaledStepper* stepper;

				//Default acceleration
				double defAccel = 40;

				//Default max speed
				double defMax = 70;

				//Current tower position
				int targetTowerPos = 0;

				//Tower positions
				double towerPos[4] = {0, 90, 180, 270};

				//Carry offset
				double carryOffset = 45;

				double convertToDegree(double raw);
				double convertToRaw(double degree);

				int numPos();
				double localize(double globalAngle);
			public:
				Turret(double stepsPerDegree, ScaledStepper* stepper);

				double distanceToGo();
				void wait();

				void home();

				double currentPosition();
				double currentPosition(bool global);

				double targetPosition();
				double targetPosition(bool global);
				
				double getTowerPos(int tower);
				int targetTower();

				double getStepError();

				int closestTower();

				int nextTower(int change);
				int nextTower(int curr, int change);

				int nextTowerTo(int target);
				int nextTowerTo(int curr, int target);

				int prevTowerTo(int target);
				int prevTowerTo(int curr, int target);

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
				
				bool isRunning();
				void wait();

				bool toggle();
		};

		class ColorSensor {
			private:
				//Color sensor object
				Adafruit_TCS34725 tcs;

				//Empty/block present threshold
				int emptyThres = 120;

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
				
				//Interval to use
				int setInterval = 0;

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

				//Time of synchronization start
				unsigned long syncStart = 0;

				//Timestamp for last received signal
				unsigned long timestamp = 0;

				//Whether signals for other addresses are automatically relayed
				bool autoRelay = false;
				
				void unpack(unsigned int address, unsigned int command);
			public:
				IRT(int address, int sendPin, int recvPin);

				void begin();

				int getAddress();

				void setSendActive(bool active);

				void send(unsigned int address, unsigned int command);
				void send(unsigned int address, unsigned int command, unsigned int data);

				void setSendInterval(int interval);

				void setSendRepeats(int repeats);
				void resetSendRepeats();

				void useInterval();

				bool isSending();
				void waitSend();

				void setReceiveActive(bool active);

				bool receive(unsigned int*command, unsigned int*data);
				bool receive(unsigned int*address, unsigned int*command, unsigned int*data);

				void waitReceive();
				bool waitReceive(int timeout);

				unsigned long getTimestamp();

				void setAutoRelay(bool active);

				void update();

				void synchronize();
				void beginSync();
				void waitSync(int channels, int size);
				void updateSync(unsigned long timestamp, int size);
		};

		TowerRobot(Slide* slide, Turret* turret, Gripper* gripper);
		TowerRobot(Slide* slide, Turret* turret, Gripper* gripper, ColorSensor* colorSensor);
		TowerRobot(Slide* slide, Turret* turret, Gripper* gripper, ColorSensor* colorSensor, IRT* irt);

		void setTowerHeights(int tower1, int tower2, int tower3, int tower4);
		int getTowerHeight(int tower);

		bool waitSlideTurret();

		void sleep(unsigned long timeout);

		void home();
		void home(double homePos);

		bool moveToBlock(int tower);
		bool moveToBlock(int tower, double blockNum);

		bool load(int tower);
		bool load(int tower, int blockNum);

		bool unload(int tower);

		int getCargo();

		int scanBlock(int tower, int blockNum);

		void synchronize();
		
		void setAutoRelay(bool active);

		void beginYield();
		void endYield();
		
		void sendYield();
		void sendDone();

		bool updateYield();

		void remoteControl();

		int findHeight(int tower);
		int findHeight(int tower, int* bufferColors);
		int scanTower(int tower, int color, bool* startedTarget, int* bufferColors);
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

		//Whether movements are yielded to other robots
		bool yieldActive = false;

		//Yielding mode
		int yieldMode = DORMANT;

		//Block heights of each tower
		int towerHeights[4] = {0, 0, 0, 0};

		//Current number of block cargo
		int cargo = 0;

		//Target turret position
		int turretTarget = 0;

		//Target slide position
		int slideTarget = 0;

		//Margin for color sensor to read block
		double sensorMargin = 0.3;

		//Turret angle tracker
		double turretAngle = 0;

		//Angle to send signals at (0 - 90)
		double sendAngle = 30;
};

#endif