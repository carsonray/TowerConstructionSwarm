// Include the TowerRobot Library
#include <TowerRobot.h>

// Creates a color sensor instance
TowerRobot::ColorSensor sensor = TowerRobot::ColorSensor();

void setup() {
  Serial.begin(9600);
  sensor.begin();
}

void loop() {
  switch(sensor.getBlockColor()) {
    case EMPTY:
      Serial.println("Empty");
      break;
    case BLACK:
      Serial.println("Black");
      break;
    case WHITE:
      Serial.println("White");
      break;
    case RED:
      Serial.println("Red");
      break;
    case BLUE:
      Serial.println("Blue");
      break;
  }
}