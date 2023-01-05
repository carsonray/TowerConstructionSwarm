// Include the Slide Library
#include <Slide.h>

// Define parameters
const int stepsPerBlock = 222;
const int stepPin 12
const int dirPin 13
const int[] modePins = {9, 10, 11}

// Creates an instance
Slide slide = Slide(stepsPerBlock, stepPin, dirPin, modePins);

void setup() {
  
}

void loop() {
  	slide.moveToBlock(1);
  	delay(5000);
  	slide.moveToBlock(-1);
  	delay(5000);
}