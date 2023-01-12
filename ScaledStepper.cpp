/*
  ScaledStepper.h - Controls microstepping modes on A4988 Driver extending AccelStepper class
  Created by Carson G. Ray, January 4 2022.
*/

#include "Arduino.h"
#include "AccelStepper.h"
#include "ScaledStepper.h"

//Constructs stepper object on A4988 pins
ScaledStepper::ScaledStepper(int step, int dir, int mode1, int mode2, int mode3) : AccelStepper(1, step, dir) {
	//Initializes step mode pins
    this->modePins[0] = mode1;
	this->modePins[1] = mode2;
	this->modePins[2] = mode3;

	for (int modePin: this->modePins) {
		pinMode(modePin, OUTPUT);
	}
}

//Sets steps per unit of external scale
void ScaledStepper::setScale(float scaleFactor) {
    this->scaleFactor = scaleFactor;
}

//Converts external unit to steps
float ScaledStepper::s(float input) {
    return input*scaleFactor;
}

//Converts steps to external unit
float ScaledStepper::_s(float output) {
    return output/scaleFactor;
}

//Sets limits of step speed before step mode is changed
void ScaledStepper::setSpeedRange(float minSpeed, float maxSpeed) {
    //Auto enables mode switching
    enableModeSwitch();

    /*
    Ensures that the ratio of the bounds is greater
    than or equal to two to prevent oscillation
    */ 
    if (maxSpeed < minSpeed*2) {
        maxSpeed = minSpeed*2;
    }

    speedRange[0] = minSpeed;
    speedRange[1] = maxSpeed;
}

//Sets range of permitted microstepping speeds
void ScaledStepper::setModeRange(int minMode, int maxMode) {
    enableModeSwitch();
    modeRange[0] = minMode;
    modeRange[1] = maxMode;
}

/*
Enables dynamic switching of microstepping modes
If step speed falls outside range, step mode will be adjusted
to stay inside range
Note: Bounds must have a ratio greater than or equal to 2
*/
void ScaledStepper::enableModeSwitch() {
    enableModeSwitch(true);
}
void ScaledStepper::enableModeSwitch(bool useModeSwitch) {
    this->useModeSwitch = useModeSwitch;
}

//Sets A4988 microstepping mode pins
void ScaledStepper::setModePins(bool mode1, bool mode2, bool mode3) {
    digitalWrite(modePins[0], mode1);
    digitalWrite(modePins[1], mode2);
    digitalWrite(modePins[2], mode3);
}

//Sets microstepping mode  (ex. 4 is quarter stepping)
void ScaledStepper::setStepMode(int stepMode) {
    switch (stepMode) {
        case 2:
            setModePins(HIGH, LOW, LOW);
            break;
        case 4:
            setModePins(LOW, HIGH, LOW);
            break;
        case 8:
            setModePins(HIGH, HIGH, LOW);
            break;
        case 16:
            setModePins(HIGH, HIGH, HIGH);
            break;
        default:
            stepMode = 1;
            setModePins(LOW, LOW, LOW);
            break;
    }

    //Sets step mode
    int prevMode = this->stepMode;
    this->stepMode = stepMode;

    //Resets full step integration and stepper settings
    resetTracking(prevMode, stepMode);
}

int ScaledStepper::getStepMode() {
    return stepMode;
}

/*
Resets full step integration
Updates counters so that appropriate number of microsteps
add one full step to net position
*/
void ScaledStepper::resetTracking(int prevMode, int currMode) {
    //Updates current position
    prevRawPos = AccelStepper::currentPosition();

    //Updates stepper settings for mode
    double scaleFactor = ((double) currMode)/prevMode;

    AccelStepper::setSpeed(unscaleVal(AccelStepper::speed(), scaleFactor));
    AccelStepper::setMaxSpeed(unscaleVal(AccelStepper::maxSpeed(), scaleFactor));
    AccelStepper::setAcceleration(unscaleVal(AccelStepper::acceleration(), scaleFactor));
    Serial.println(AccelStepper::targetPosition());
    AccelStepper::moveTo(unscalePos(AccelStepper::targetPosition(), scaleFactor));
    Serial.println(AccelStepper::targetPosition());
}

/*
Runs stepper one step
*/
boolean ScaledStepper::run() {
    if (useModeSwitch) {
        //Gets raw step speed
        float rawSpeed = abs(AccelStepper::speed());
        
        if ((rawSpeed < speedRange[0]) && (stepMode < modeRange[1])) {
            //Moves to lower microstepping mode to increase step speed
            setStepMode(stepMode*2);
        } else if ((rawSpeed > speedRange[1]) && (stepMode > modeRange[0])) {
            //Moves to higher microstepping mode to decrease step speed
            setStepMode(stepMode/2);
        }
    }
    
    return AccelStepper::run();
}

//Scales raw step position to integrated full step position
double ScaledStepper::scalePos(long rawPos) {
    //Uses zero positions at last mode change and integrates
    return (rawPos-prevRawPos)/((double) stepMode) + prevScaledPos;
}

//Unscales intergrated full steps to raw step position
long ScaledStepper::unscalePos(double scaledPos) {
    return unscalePos(scaledPos, stepMode);
}
long ScaledStepper::unscalePos(double scaledPos, double scaleFactor) {
    //Uses zero positions at last mode change and integrates
    return (long) ((scaledPos-prevScaledPos)*scaleFactor + prevRawPos);
}

//Scales value to terms of full steps
double ScaledStepper::scaleVal(double raw) {
    return raw/stepMode;
}

//Unscales value to terms of raw steps
double ScaledStepper::unscaleVal(double scaled) {
    return unscaleVal(scaled, stepMode);
}
double ScaledStepper::unscaleVal(double scaled, double scaleFactor) {
    return scaled*scaleFactor;
}

//Gets current full step position
double ScaledStepper::currentPosition() {
    return scalePos(AccelStepper::currentPosition());
}

//Gets current full step target position
double ScaledStepper::targetPosition() {
    return scalePos(AccelStepper::targetPosition());
}

//Gets full steps to target position
double ScaledStepper::distanceToGo() {
    return scaleVal(AccelStepper::distanceToGo());
}

//Resets full step position
void ScaledStepper::setCurrentPosition(double position) {
    AccelStepper::setCurrentPosition(0);
    prevRawPos = 0;
    prevScaledPos = position;
}

//Runs one step unless full step position is reached
void ScaledStepper::runToNewPosition(double position) {
    AccelStepper::runToNewPosition(unscalePos(position));
}

//Moves to absolute full step position
void ScaledStepper::moveTo(double absolute) {
    AccelStepper::moveTo(unscalePos(absolute));
}

//Moves relatively by full steps
void ScaledStepper::move(double relative) {
    AccelStepper::move((long) unscaleVal(relative));
}

//Gets speed in full steps per second
float ScaledStepper::speed() {
    return (float) scaleVal(AccelStepper::speed());
}

//Gets max full step per second speed
float ScaledStepper::maxSpeed() {
    return (float) scaleVal(AccelStepper::maxSpeed());
}

//Sets full steps per second speed
void ScaledStepper::setSpeed(float speed) {
    AccelStepper::setSpeed((float) unscaleVal(speed));
}

//Sets max full steps per second speed
void ScaledStepper::setMaxSpeed(float speed) {
    AccelStepper::setMaxSpeed((float) unscaleVal(speed));
}

//Gers acceleration in full steps per second squared
float ScaledStepper::acceleration() {
    return (float) scaleVal(AccelStepper::acceleration());
}

//Sets full steps per second squared acceleration
void ScaledStepper::setAcceleration(float acceleration) {
    AccelStepper::setAcceleration((float) unscaleVal(acceleration));
}