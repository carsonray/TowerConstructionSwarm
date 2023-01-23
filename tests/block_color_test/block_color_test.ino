// Include the TowerRobot Library
#include <TowerRobot.h>

// Creates a color sensor instance
TowerRobot::ColorSensor sensor = TowerRobot::ColorSensor();

void setup() {
  Serial.begin(9600);
  sensor.begin();
}

void loop() {
  Serial.println(sensor.getBlockColor());
}