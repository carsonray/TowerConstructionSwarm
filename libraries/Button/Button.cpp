/*
  Button.cpp - Library for handling button events and debouncing
  Created by Carson G. Ray, August 12 2022.
  Released into the public domain.
*/

#include "Arduino.h"
#include "Button.h"

Button::Button(int pin) {
	//Sets button input
	this->pin = pin;
	pinMode(pin, INPUT);

	//Sets debounce and pulse start times
	bounceStart = millis();
	pulseStart = millis();
	prevStart = millis();
}

Button::Button(int pin, bool pullup) : Button(pin) {
	//Sets pullup/pulldown state
	this->pullup = pullup;
}

Button::Button(int pin, int debounce, bool pullup=false): Button(pin, pullup) {
	//Sets debounce length
	this->debounce = debounce;
}

void Button::update() {
	//Updates button states

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

void Button::updatePulse() {
	//Updates pulse times if fully debounced
	if (change()) {
		//Sets previous pulse start
		prevStart = pulseStart;
		
		//Resets to beginning of debounce
		pulseStart = bounceStart;
	}
}

// Button event functions
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

bool Button::changeTo(bool target) {
	return changeTo(target, true);
}

bool Button::changeTo(bool target, bool bounce) {
	//If state has just changed to target
	return change(bounce) && (state(bounce) == target);
}

int Button::pulseTime() {
	return pulseTime(true);
}

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

int Button::pulse() {
	//Returns previous pulse length
	return pulseStart - prevStart;
}

int Button::pulse(bool target) {
	//Returns previous pulse length if it was target state
	if (state() != target) {
		return pulse();
	} else {
		return 0;
	}
}
