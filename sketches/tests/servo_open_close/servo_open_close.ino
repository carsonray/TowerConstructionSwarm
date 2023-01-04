  #include <Servo.h>
#include <Button.h>

//Servo pin
#define servoPin 5

//Button pin
#define buttonPin 6

//Whether gripper is open
boolean openState = true;

Servo servo;

//Creates button
Button button = Button(buttonPin, 10);

void setup() {
  //Attaches servo
  Serial.begin(9600);
  servo.attach(servoPin);
}

void loop() {
  Serial.print(digitalRead(buttonPin));
  Serial.print(button.state(false));
  Serial.println(button.state());
  if (button.changeTo(LOW)) {
    openState = !openState;
    if (openState) {
      Serial.println("Open");
    } else {
      Serial.println("Closed");
    }
  }

  if (openState) {
    servo.write(50);
  } else {
    servo.write(175);
  }

  button.update();
}
