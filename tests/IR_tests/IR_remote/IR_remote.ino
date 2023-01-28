/*
 * IRremote: IRsendDemo - demonstrates sending IR codes with IRsend
 * An IR LED must be connected to Arduino PWM pin 3.
 * Version 0.1 July, 2009
 * Copyright 2009 Ken Shirriff
 * http://arcfn.com
 */


#include <IRremote.hpp>

#define IR_SEND_PIN 3

void setup()
{
	IrSender.begin(ENABLE_LED_FEEDBACK);
}

void loop() {
	IrSender.sendNEC(0x00, 4294967295, 2);
	delay(5000); //5 second delay between each signal burst
}