#include <Servo.h>

//Servo pin
#define servoPin 0

Servo servo;

void setup() {
  //Attaches servo
  servo.attach(servoPin);
}

void loop() {
  servo.write(172);
  delay(10000);
  servo.write(42);
  delay(10000);
}
