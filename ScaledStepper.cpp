/*
  ScaledStepper.h - Controls microstepping modes on A4988 Driver extending AccelStepper class
  Created by Carson G. Ray, January 4 2022.
*/

#include <Arduino.h>
#include "AccelStepper.h"
#include "ScaledStepper.h"
#include "Utils.h"

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

//Sets A4988 microstepping mode pins
void ScaledStepper::setModePins(bool mode1, bool mode2, bool mode3) {
    digitalWrite(modePins[0], mode1);
    digitalWrite(modePins[1], mode2);
    digitalWrite(modePins[2], mode3);
}

//Sets microstepping mode  (ex. 4 is quarter stepping)
void ScaledStepper::setStepMode(int stepMode) {
    //Limits step modes
    if (stepMode > modeRange[1]) {
        setStepMode(modeRange[1]);
        return;
    } else if (stepMode < modeRange[0]) {
        setStepMode(modeRange[0]);
        return;
    }

    switch (stepMode) {
        case 1:
            setModePins(LOW, LOW, LOW);
            break;
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
    }

    //Resets full step integration and stepper settings
    resetTracking();

    //Sets step mode
    this->stepMode = stepMode;
}

//Gets step mode
int ScaledStepper::getStepMode() {
    return stepMode;
}

/*
Resets full step integration
Updates counters so that appropriate number of microsteps
add one full step to net position
*/
void ScaledStepper::resetTracking() {
    //Updates current position
    prevScaledPos = currentPosition();
    prevRawPos = AccelStepper::currentPosition();
}

//Scales raw step position to integrated full step position
double ScaledStepper::scalePos(long rawPos) {
    //Uses zero positions at last mode change and integrates
    return (rawPos-prevRawPos)/((double) stepMode) + prevScaledPos;
}

//Unscales intergrated full steps to raw step position
long ScaledStepper::unscalePos(double scaledPos) {
    //Uses zero positions at last mode change and integrates
    return round((scaledPos-prevScaledPos)*stepMode + prevRawPos);
}

//Scales value to terms of full steps
double ScaledStepper::scaleVal(double raw) {
    return raw/stepMode;
}

//Unscales value to terms of raw steps
double ScaledStepper::unscaleVal(double scaled) {
    return scaled*stepMode;
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
    moveTo(currentPosition() + relative);
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