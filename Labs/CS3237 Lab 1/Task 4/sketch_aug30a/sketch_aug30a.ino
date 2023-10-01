#define PIN 4
#define DEBOUNCE_TIME 100

volatile bool pressed = false;
volatile int pressCount = 0;

void IRAM_ATTR isr() {
  static volatile unsigned long lastInterruptTime = 0;
  volatile unsigned long interruptTime = millis();
  if ((interruptTime - lastInterruptTime) > DEBOUNCE_TIME) {
    pressCount++;
    pressed = true;
  }
  lastInterruptTime = interruptTime;
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN, INPUT_PULLUP);
  attachInterrupt(PIN, isr, FALLING);
}

void loop() {
  if (pressed) {
    Serial.printf("Button has been pressed %u times\n", pressCount);
    pressed = false;
  }
}