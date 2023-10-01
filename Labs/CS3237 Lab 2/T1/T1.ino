// #include <ESP32Servo.h>
// int APin = 13;
// ESP32PWM pwm;
// int freq = 1000;
// void setup() {
// 	// Allow allocation of all timers
// 	ESP32PWM::allocateTimer(0);
// 	ESP32PWM::allocateTimer(1);
// 	ESP32PWM::allocateTimer(2);
// 	ESP32PWM::allocateTimer(3);
// 	Serial.begin(115200);
// 	pwm.attachPin(APin, freq, 10); // 1KHz 8 bit

// }
// void loop() {
  //0-0.5-0: fast move CW and CCW
  // 0.5-1.0-0.5: slow move CCW and CW

	// fade the LED on thisPin from off to brightest:
	// for (float brightness = 0; brightness <= 0.5; brightness += 0.001) {
	// 	// Write a unit vector value from 0.0 to 1.0
  //   Serial.println("up");
	// 	pwm.writeScaled(brightness);
	// 	delay(2);
	// }

  // for (float brightness = 0.5; brightness >= 0; brightness -= 0.001) {
  //   Serial.println("down");
	// 	pwm.writeScaled(brightness);
	// 	delay(2);
	// }
  
	// delay(1000);
	// fade the LED on thisPin from brithstest to off:
	// for (float brightness = 0.5; brightness >= 0; brightness -= 0.001) {
	// 	freq += 10;
	// 	// Adjust the frequency on the fly with a specific brightness
	// 	// Frequency is in herts and duty cycle is a unit vector 0.0 to 1.0
	// 	pwm.adjustFrequency(freq, brightness); // update the time base of the PWM
  //   Serial.print(freq);
  //   Serial.print(" || ");
  //   Serial.println(brightness);
	// 	delay(2);
	// }
	// // // pause between LEDs:
	// delay(1000);
	// freq = 1000;
	// pwm.adjustFrequency(freq, 0.0);    // reset the time base
// }


#include <ESP32Servo.h>
Servo myservo;
int pos = 0;
int servoPin = 13;

void setup() {
	// Allow allocation of all timers
	ESP32PWM::allocateTimer(0);
	ESP32PWM::allocateTimer(1);
	ESP32PWM::allocateTimer(2);
	ESP32PWM::allocateTimer(3);
	myservo.setPeriodHertz(50);    // standard 50 hz servo
	myservo.attach(servoPin, 1000, 2000); // attaches the servo on pin 18 to the servo object
	// using default min/max of 1000us and 2000us
	// different servos may require different min/max settings
	// for an accurate 0 to 180 sweep
}

void loop() {

	for (pos = 0; pos <= 90; pos += 2) {
    // for left to centre
		myservo.write(pos);    
		delay(15);             
	}
  delay(1000);
  for (pos = 90; pos <= 180; pos += 2) {
		myservo.write(pos);    
		delay(15);             
	}
  delay(1000);
  for (pos = 180; pos >= 90; pos -= 2) {
		myservo.write(pos);    
		delay(15);             
	}
  delay(1000);
	for (pos = 90; pos >= 0; pos -= 2) { 
		myservo.write(pos);    
		delay(15);             
	}
  delay(1000);
}