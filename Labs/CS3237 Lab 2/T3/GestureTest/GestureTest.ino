#include <Wire.h>
#include <SparkFun_APDS9960.h>

#include <ESP32Servo.h>

// Pins
#define APDS9960_INT 

// Global Variables
SparkFun_APDS9960 apds = SparkFun_APDS9960();
volatile int isr_flag = 0;
Servo myservo;  // create servo object to control a servo
int servoPin = 13;

void setup() {
  // Initialize Serial port
  myservo.attach(servoPin);
  Serial.begin(9600);
  Serial.println();
  Serial.println(F("--------------------------------"));
  Serial.println(F("SparkFun APDS-9960 - GestureTest"));
  Serial.println(F("--------------------------------"));


  // Initialize APDS-9960 (configure I2C and initial values)
  if ( apds.init() ) {
    Serial.println(F("APDS-9960 initialization complete"));
  } else {
    Serial.println(F("Something went wrong during APDS-9960 init!"));
  }
  
  // Start running the APDS-9960 gesture sensor engine
  if ( apds.enableGestureSensor(true) ) {
    Serial.println(F("Gesture sensor is now running"));
  } else {
    Serial.println(F("Something went wrong during gesture sensor init!"));
  }
}

void loop() {
      handleGesture();
}

void handleGesture() {
    if ( apds.isGestureAvailable() ) {
    switch ( apds.readGesture() ) {
      case DIR_UP:
        Serial.println("UP");
        break;
      case DIR_DOWN:
        Serial.println("DOWN");
        break;
      case DIR_LEFT:
        Serial.println("LEFT");
        myservo.write(0);
        break;
      case DIR_RIGHT:
        Serial.println("RIGHT");
        myservo.write(180);
        break;
      case DIR_NEAR:
        Serial.println("NEAR");
        break;
      case DIR_FAR:
        Serial.println("FAR");
        break;
      default:
        Serial.println("NONE");
    }
  }
}