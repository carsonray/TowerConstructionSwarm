#include <IRremote.hpp>

#define SEND_PIN 13

void setup()
{
	IrSender.begin(SEND_PIN, DISABLE_LED_FEEDBACK, USE_DEFAULT_FEEDBACK_LED_PIN);
}

void loop() {
	IrSender.sendNEC(0xB1, 0xA2, 2);
	delay(1000); //5 second delay between each signal burst
}