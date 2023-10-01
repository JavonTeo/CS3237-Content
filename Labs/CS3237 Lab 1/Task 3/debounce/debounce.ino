#define SWITCH 4
#define DEBOUNCE_TIME 50

byte state = LOW;
int pressedTime = 0;

void setup() {
  Serial.begin(9600);
  pinMode(SWITCH, INPUT_PULLUP);
}
void loop() {
 if (digitalRead(SWITCH) == LOW) {
   if ((millis() - pressedTime) > DEBOUNCE_TIME) {
     state = !state;
     pressedTime = millis();
   }
  }
 if (state) {
   Serial.println("Toggle On");
  } else {
    Serial.println("Toggle Off");
  }
}