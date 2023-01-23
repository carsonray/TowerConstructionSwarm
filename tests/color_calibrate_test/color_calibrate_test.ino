#include <TowerRobot.h>

/* Connect SCL    to analog 5
   Connect SDA    to analog 4
   Connect VDD    to 3.3V DC
   Connect GROUND to common ground
   Connect LED to INT
*/

/* Initialise with specific int time and gain values */
TowerRobot::ColorSensor sensor = TowerRobot::ColorSensor();

void setup(void) {
  Serial.begin(9600);

  if (sensor.begin()) {
    Serial.println("Found sensor");
  } else {
    Serial.println("No TCS34725 found ... check your connections");
    while (1);
  }
}

void loop(void) {
  uint16_t r, g, b, c;

  sensor.getReflected(&r, &g, &b, &c);

  Serial.print("R: "); Serial.print(r, DEC); Serial.print(" ");
  Serial.print("G: "); Serial.print(g, DEC); Serial.print(" ");
  Serial.print("B: "); Serial.print(b, DEC); Serial.print(" ");
  Serial.print("C: "); Serial.print(c, DEC); Serial.print(" ");
  Serial.println(" ");
}
