/*
 * IRremote: IRrecvDemo - demonstrates receiving IR codes with IRrecv
 * An IR detector/demodulator must be connected to the input RECV_PIN.
 * Version 0.1 July, 2009
 * Copyright 2009 Ken Shirriff
 * http://arcfn.com
 */

#include <IRremote.hpp>

#define RECV_PIN 5

void setup()
{
  Serial.begin(9600);
  // In case the interrupt driver crashes on setup, give a clue
  // to the user what's going on.
  Serial.println("Enabling IRin");
  IrReceiver.begin(RECV_PIN, DISABLE_LED_FEEDBACK);
  Serial.println("Enabled IRin");
}

void loop() {
  if (IrReceiver.decode()) {
    Serial.println(IrReceiver.decodedIRData.address, HEX);
    Serial.println(IrReceiver.decodedIRData.command, HEX);
    IrReceiver.resume(); // Receive the next value
  }
}