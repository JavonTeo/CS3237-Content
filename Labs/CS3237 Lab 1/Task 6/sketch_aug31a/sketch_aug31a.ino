#define PIN 5
unsigned long last_event = 0;

void setup() {
  pinMode(PIN, INPUT); 
  Serial.begin(115200);
}

void loop() {
  int output = digitalRead(PIN);
  if (output == LOW) {
    if (millis() - last_event > 25) {
      Serial.println("Clap sound was detected!");
    }
    last_event = millis();
  }
}