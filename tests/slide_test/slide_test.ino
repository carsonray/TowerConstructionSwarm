// Include the TowerRobot Library
#include <TowerRobot.h>
#include <ScaledStepper.h>
#include <Button.h>

// Define parameters
const int stepsPerBlock = -222;
const int upperLimit = 6;

const int stepPin = 12;
const int dirPin = 13;
const int modePins[3] = {9, 10, 11};

const int limitPin = 8;

// Creates scaled stepper
ScaledStepper stepper = ScaledStepper(stepPin, dirPin, modePins[0], modePins[1], modePins[2]);

// Creates a limit switch
Button limit = Button(8);

// Creates a slide instance
TowerRobot::Slide slide = TowerRobot::Slide(stepsPerBlock, upperLimit, &stepper, &limit);

void setup() {
  slide.home(0);
}

void loop() {
  
}