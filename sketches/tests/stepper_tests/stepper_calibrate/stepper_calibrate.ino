// Define pin connections
#define stepPin 2

void setup() {
  pinMode(stepPin, OUTPUT);
}

void loop() {
  //Set step pin to high without clocking
  digitalWrite(stepPin, HIGH);
}
