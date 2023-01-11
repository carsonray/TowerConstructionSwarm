// Include the Slide Library
#include <TowerRobot.h>

// Define parameters
const int stepsPerBlock = 222;
const int stepPin = 12;
const int dirPin = 13;
const int modePins[3] = {9, 10, 11};

int dir = 1;

// Creates an instance
TowerRobot::Slide slide = TowerRobot::Slide(stepsPerBlock, stepPin, dirPin, modePins[0], modePins[1], modePins[2]);

void setup() {
  slide.moveToBlock(dir);
}

void loop() {
  if (slide.distanceToGo() == 0) 
    slide.moveToBlock(-dir);

  slide.update();
}