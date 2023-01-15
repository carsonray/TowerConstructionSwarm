// Include the TowerRobot Library
#include <TowerRobot.h>
#include <ScaledStepper.h>
#include <Button.h>

// Define parameters
const double stepsPerBlock = -200/90*50;
const double upperLimit = 6;

#define stepPin 12
#define dirPin 13
const int modePins[3] = {9, 10, 11};

#define limitPin 8

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