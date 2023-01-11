/*
  Button.cpp - Library for handling button events and debouncing
  Created by Carson G. Ray, August 12 2022.
  Released into the public domain.
*/

#include "Arduino.h"
#include "Button.h"

//Sets button input
Button::Button(int pin) {
	this->pin = pin;
	pinMode(pin, INPUT);

	//Sets debounce and pulse start times
	bounceStart = millis();
	pulseStart = millis();
	prevStart = millis();
}

//Sets pullup/pulldown state
Button::Button(int pin, bool pullup) : Button(pin) {
	this->pullup = pullup;
}

//Sets debounce length
Button::Button(int pin, int debounce, bool pullup=false): Button(pin, pullup) {
	this->debounce = debounce;
}

//Updates button states
void Button::update() {
	//Updates pulse times and fallback state if fully debounced
	updatePulse();

	if (change()) {
		//Sets fallback to current state
		fallback = state();
	}

	//Updates button states

	//Sets previous state to current state
	prevstate = rawstate;

	//Updates current state (negates if pullup)
	rawstate = (digitalRead(pin) != pullup);

	//Updates debounced state

	//Resets debounce start if the raw state changes
	if (change(false)) {
		bounceStart = millis();
	}

	//If debounce time has elapsed, update debounce state
	if (pulseTime(false) >= debounce) {
		bouncestate = rawstate;
	} else {
		//Otherwise, drop back to fallback
		bouncestate = fallback;
	}
}

//Updates pulse times if fully debounced
void Button::updatePulse() {
	if (change()) {
		//Sets previous pulse start
		prevStart = pulseStart;
		
		//Resets to beginning of debounce
		pulseStart = bounceStart;
	}
}

// Button event functions

// Current button state
bool Button::state() {
	//Default is using debounce
	return state(true);
}

bool Button::state(bool bounce) {
	if (bounce) {
		//Gives debounced button state
		return bouncestate;
	} else {
		//Gives raw button state
		return rawstate;
	}
}

//If button state has changed
bool Button::change() {
	return change(true);
}

bool Button::change(bool bounce) {
	if (bounce) {
		//If debounced state has changed
		return (state() != fallback);
	} else {
		//If raw state has changed
		return (state(false) != prevstate);
	}
}

//If state has just changed to target
bool Button::changeTo(bool target) {
	return changeTo(target, true);
}

bool Button::changeTo(bool target, bool bounce) {
	return change(bounce) && (state(bounce) == target);
}

int Button::pulseTime() {
	return pulseTime(true);
}

//Elapsed time of current pulse
int Button::pulseTime(bool bounce) {
	//Updates pulse times
	updatePulse();

	if (bounce) {
		//Return debounced pulse time
		return millis() - pulseStart;
	} else {
		//Return raw pulse time
		return millis() - bounceStart;
	}
}

//Returns previous pulse length
int Button::pulse() {
	return pulseStart - prevStart;
}

//Returns previous pulse length if it was target state
int Button::pulse(bool target) {
	if (state() != target) {
		return pulse();
	} else {
		return 0;
	}
}
