// Include the ScaledStepper Library
#include <ScaledStepper.h>

// Define parameters
const int stepPin = 12;
const int dirPin = 13;
const int modePins[3] = {9, 10, 11};

const int STEPS_PER_REV = 200;

// Creates a scaled stepper instance
ScaledStepper myStepper(stepPin, dirPin, modePins[0], modePins[1], modePins[2]);

void setup() {
  // set the maximum speed, acceleration factor,
  // initial speed and the target position
  myStepper.setMaxSpeed(1000);
  myStepper.setAcceleration(50);
  myStepper.setSpeed(200);
  myStepper.moveTo(STEPS_PER_REV);
}

void loop() {
  // Change direction once the motor reaches target position
  if (myStepper.distanceToGo() == 0) 
    myStepper.moveTo(-myStepper.currentPosition());

  // Move the motor one step
  myStepper.run();
}