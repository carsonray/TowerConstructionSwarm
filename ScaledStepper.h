/*
  ScaledStepper.h - Controls microstepping modes on A4988 Driver extending AccelStepper class
  Created by Carson G. Ray, January 4 2022.
*/

#ifndef ScaledStepper_h
#define ScaledStepper_h

#include "Arduino.h"
#include "AccelStepper.h"

class ScaledStepper : public AccelStepper {
    private:
        //A4988 pins to change microstepping modes
        int modePins[3];

        //Steps per unit of external scale
		float scaleFactor = 1;

        //Limits of step speed before step mode is changed
        float speedRange[2] = {80, 180};

        //Range of permited microstepping modes
        int modeRange[2] = {1, 16};

        //Current microstepping mode
        int stepMode = 1;

        //Whether mode switching is enabled
        bool useModeSwitch = false;

        //Raw net steps travelled when step mode was last changed
        long prevRawPos = 0;

        /*
        Full steps travelled when step mode was last changed
        (ex. 4 steps count as 1 full step in quarter stepping mode)
        */
        double prevScaledPos = 0;
        
        void setModePins(bool mode1, bool mode2, bool mode3);

        double scalePos(long rawPos);
        long unscalePos(double scaledPos);

        double scaleVal(double raw);
        double unscaleVal(double scaled);
	public:
    	ScaledStepper(int step, int dir, int mode1, int mode2, int mode3);

        void setScale(float scaleFactor);
        float s(float input);
        float _s(float output);

        void setSpeedRange(float minSpeed, float maxSpeed);
        void setModeRange(int minMode, int maxMode);

        void enableModeSwitch();
        void enableModeSwitch(bool useModeSwitch);
        void setStepMode(int stepMode);

        void resetTracking();

        boolean run();

        double currentPosition();
        double targetPosition();

        double distanceToGo();
        void setCurrentPosition(double position);
        void runToNewPosition(double position);

        void moveTo(double absolute);
        void move(double relative);

        float speed();
        float maxSpeed();
        void setSpeed(float speed);
        void setMaxSpeed(float speed);

        float acceleration();
        void setAcceleration(float acceleration);
};

#endif