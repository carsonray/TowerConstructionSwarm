/*
  Slide.h - Controls vertial rack and pinion drive
  Created by Carson G. Ray, January 4 2022.
*/

#ifndef Slide_h
#define Slide_h

#include "Arduino.h"
#include "AccelStepper.h"

class Slide {
	private:
		AccelStepper stepper;
		int stepsPerBlock;
		int stepPin;
		int dir;
		int modePins[3];
	public:
    	Slide(int spb, int step, int dir, int*modePins);

		void moveToBlock(int num);
};

#endif