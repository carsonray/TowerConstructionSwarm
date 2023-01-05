/*
  Button.h - Library for handling button events and debouncing
  Created by Carson G. Ray, August 12 2022.
*/

#ifndef Button_h
#define Button_h

#include "Arduino.h"

class Button {
	private:
		//Button input pin
		int pin;
		
		//Debounce time
		int debounce = 3;

		//Raw button state
		bool rawstate = LOW;

		//Previous button state
		bool prevstate = LOW;

		//Debounced button state
		bool bouncestate = LOW;

		//Fallback state for debounce
		bool fallback = LOW;

		//Pullup/pulldown state
		bool pullup = false;

		//Current debounce start time
		int bounceStart;

		//Previous pulse start time
		int prevStart;

		//Current pulse start time
		int pulseStart;

		void updatePulse();

	public:
    	Button(int _pin);
		Button(int _pin, bool _pullup);    
		Button(int _pin, int _debounce, bool _pullup=false);

		void update();

		bool state();
		bool state(bool bounce);

		bool change();
		bool change(bool bounce);

		bool changeTo(bool target);
		bool changeTo(bool target, bool bounce);

		int pulseTime();
		int pulseTime(bool bounce);

		int pulse();
		int pulse(bool target);
};

#endif
