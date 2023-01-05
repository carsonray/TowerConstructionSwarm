/*
  Slide.cpp - Controls vertial rack and pinion drive
  Created by Carson G. Ray, January 4 2022.
*/

#include "Arduino.h"
#include "AccelStepper.h"
#include "Slide.h"

Slide::Slide(int spb, int step, int dir, int*modePins) {
	this->stepsPerBlock = spb;
	this->stepPin = step;
	this->dirPin = dir;
	this->modePins = *(modePins);

	stepper = AccelStepper(1, stepPin, dirPin);

	for (int modePin: this->modePins) {
		pinMode(modePin, OUTPUT);
	}

	digitalWrite(this->modePins[1], HIGH);
}

void Slide::moveToBlock(int num) {
	stepper.setMaxSpeed(1000);
  	stepper.setAcceleration(50);
  	stepper.setSpeed(200);
  	stepper.moveTo(num*stepsPerBlock);
}