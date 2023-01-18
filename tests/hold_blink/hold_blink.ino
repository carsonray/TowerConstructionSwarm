/*
  hold_blink.ino - Speeds up light blinking the longer the button is held
  Created by Carson G. Ray, August 12 2022.
  Released into the public domain.
*/

#include <Button.h>
#include <math.h>

//Button pin
#define HOLD 8

//Light pin
#define BLINK 13

//Initial time of light blink
int initial = 2000;

//Exponential rate of time decrease per second of pulse
float expRate = 0.8;

//Current light state
int blinkState = LOW;




//Creates button from pin with 1 second debounce time
Button hold = Button(HOLD, 1000);

//Start of current blink
int blinkStart = 0;

//Current light blink time
int blinkTime = 0;

//Current time
int mstime = 0;

//Sets light state
void light(int pin, bool out) {
  digitalWrite(pin, out);

  //Sets current state
  blinkState = out;
}

void setup() {
  pinMode(BLINK, OUTPUT);

  //Initializes light to blinkState
  digitalWrite(BLINK, blinkState);
}

void loop() {
  //Only blinks if button is pressed
  if (hold.state()) {
    //Gets clock time
    mstime = millis();
    //If ready for next blink or button has just been pressed
    if (hold.changeTo(HIGH) || ((mstime - blinkStart) >= blinkTime)) {
      //Resets blink start
      blinkStart = mstime;
      
      //Decreases blink time exponentially by second of pulse length
      blinkTime = initial*pow(expRate, hold.pulseTime()/1000);
      
      //Sets light to opposite state
      light(BLINK, !blinkState);
    }
  } else if (hold.changeTo(LOW)) {
    //Resets light state if button is released
    digitalWrite(BLINK, LOW);
    blinkState = LOW;
  }

  //Updates button events (must be done at the end)
  hold.update();
}
