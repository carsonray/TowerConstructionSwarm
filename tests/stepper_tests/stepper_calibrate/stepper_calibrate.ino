// Define pin connections
#define stepPin1 6
#define dirPin1 7
#define stepPin2 12
#define dirPin2 13

void setup() {
  pinMode(stepPin1, OUTPUT);
  pinMode(stepPin2, OUTPUT);
  pinMode(dirPin1, OUTPUT);
  pinMode(dirPin2, OUTPUT);
}

void loop() {
  //Set step pin to high without clocking
  digitalWrite(stepPin1, HIGH);
  digitalWrite(stepPin2, HIGH);
  digitalWrite(dirPin1, HIGH);
  digitalWrite(dirPin2, HIGH);
}
