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
    case -1:
      Serial.println("Empty");
      break;
    case 0:
      Serial.println("Black");
      break;
    case 1:
      Serial.println("White");
      break;
    case 2:
      Serial.println("Red");
      break;
    case 3:
      Serial.println("Blue");
      break;
  }
}