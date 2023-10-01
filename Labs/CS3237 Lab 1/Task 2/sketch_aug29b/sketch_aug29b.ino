#define LEDPIN 5
#define LEDCHANNEL 1
#define LEDFREQ 5000
#define LEDRESOLUTION 8

// PWM DutyCycle
int low_brightness = 0;
int high_brightness = 150;
int current_brightness = low_brightness;
bool increasing = true;
void setup() {
  ledcSetup(LEDCHANNEL, LEDFREQ, LEDRESOLUTION);
  ledcAttachPin(LEDPIN, LEDCHANNEL);
  ledcWrite(LEDCHANNEL, current_brightness);
}

void loop() {
  if (increasing) {
    current_brightness += 50;
  } else {
    current_brightness -= 50;
  }
  ledcWrite(LEDCHANNEL, current_brightness);
  delay(1000);
  
  if (current_brightness == high_brightness) {
    increasing = false;
  } else if (current_brightness == low_brightness) {
    increasing  = true;
  }
}