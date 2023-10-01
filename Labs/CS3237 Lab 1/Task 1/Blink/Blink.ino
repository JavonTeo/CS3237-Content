#define LED 5

// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin 5 as an output.
  pinMode(LED, OUTPUT);
  Serial.begin(9600);
  digitalWrite(LED, HIGH);  // turn the LED on (HIGH is the voltage level)
  Serial.println("on");
  delay(4000);                      // wait for 4 second
  digitalWrite(LED, LOW);   // turn the LED off by making the voltage LOW
  Serial.println("off");
  delay(4000);
  digitalWrite(LED, HIGH);  // turn the LED on (HIGH is the voltage level)
  Serial.println("on");
  delay(2000);                      // wait for 2 second
  digitalWrite(LED, LOW);   // turn the LED off by making the voltage LOW
  Serial.println("off");
  delay(2000);
}

// the loop function runs over and over again forever
void loop() {
  digitalWrite(LED, HIGH);  // turn the LED on (HIGH is the voltage level)
  Serial.println("ON");
}
