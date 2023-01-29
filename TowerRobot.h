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
	//Address accepted by all
	#define MASTER_ADDRESS 0

  #define IR_POLL_STATUS 0x00
  #define IR_STATUS_WAITING 0x01
  #define IR_STATUS_READY 0x02

  #define IR_STOP 0xFF

  #define IR_TOWER_UPDATE 0x10

  #define IR_CLOSE_GRIP 0x20
  #define IR_OPEN_GRIP 0x21
  #define IR_TURRET_LEFT 0x22
  #define IR_TURRET_RIGHT 0x23
  #define IR_SLIDE_DOWN 0x24
  #define IR_SLIDE_UP 0x25

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

				double getHomePos();

				void moveToBlock(double blockPos);
				void moveToBlock(double blockPos, double accel, double max);
				
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

		class IRT {
			private:
				//Sending pin
				int sendPin;

				//Receiving pin
				int recvPin;

				//Unique address
				int address;

				//Signal bitmap (key, command, data)
				int bitMap[3] = {2, 8, 16};

				//Whether sending is active
				bool sendActive = true;

				//Current sending address
				unsigned int sendAddress = 0;

				//Current sending key
				unsigned int sendKey = 0;
				
				//Current sending signal
				unsigned long sendSignal = 0;

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
				int sendRepeats = 1;

				//Whether receiving is active
				bool recvActive = true;

				//Whether recieved signal is availiable
				bool recvExists = false;

				//Whether recieved signal is cleared on read
				bool recvClear = true;

				//Current recieved address
				unsigned int recvAddress = 0;

				//Current recieved key
				unsigned int recvKey = 0;

				//Last recieved key
				unsigned int prevRecvKey = 0;

				//Current recieved command
				unsigned int recvCommand = 0;

				//Current recieved data
				unsigned int recvData = 0;

				//Whether signals for other addresses are automatically relayed
				bool autoRelay = false;
				
				void unpack(unsigned long signal);
			public:
				IRT(int id, int sendPin, int recvPin);

				void begin();

				void send(unsigned int address, unsigned int command);
				void send(unsigned int address, unsigned int command, unsigned int data);

				void setSendInterval(int interval);
				void setSendInterval(int minTime, int maxTime);

				void setSendRepeats(int repeats);
				void setSendActive(bool active);
				void resetSendRepeat();

				bool receive(unsigned int*command, unsigned int*data);
				bool receiveOnce(unsigned int*command, unsigned int*data);

				void resumeReceive();

				void setRecieveActive(bool active);
				void setAutoClear(bool cleared);

				void setAutoRelay(bool active);

				void update();
		};

		TowerRobot(Slide* slide, Turret* turret, Gripper* gripper);
		TowerRobot(Slide* slide, Turret* turret, Gripper* gripper, ColorSensor* colorSensor);
		TowerRobot(Slide* slide, Turret* turret, Gripper* gripper, ColorSensor* colorSensor, IRT* irt);

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
		IRT* irt;

		//Whether color sensor is initialized
		bool colorInit = false;

		//Whether infrared tranciever is initialized
		bool irtInit = false;

		//Block heights of each tower
		int towerHeights[4] = {0, 0, 0, 0};

		//Current number of block cargo
		int cargo = 0;

		//Margin to clear blocks after loading
		double clearMargin = 0.2;
};

#endif