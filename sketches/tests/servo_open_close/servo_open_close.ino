#include <Servo.h>
#include <Button.h>

//Servo pin
#define servoPin 2

//Button pin
#define buttonPin 8

//Whether gripper is open
boolean openState = true;

Servo servo;

//Creates button
Button button = Button(buttonPin, 10);

void setup() {
  //Attaches servo
  servo.attach(servoPin);
}

void loop() {
  if (button.changeTo(LOW)) {
    openState = !openState;
    if (openState) {
      Serial.println("Open");
    } else {
      Serial.println("Closed");
    }
  }

  if (openState) {
    servo.write(40);
  } else {
    servo.write(160);
  }

  button.update();
}
