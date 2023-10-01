uint8_t address = 0x39;
uint8_t MSByte = 0, LSByte = 0;
uint16_t regValue = 0;

#include <Wire.h>

void setup() {
  
  Wire.begin();
  Serial.begin(9600);

  Wire.beginTransmission(address);
  Wire.write(0x80);
  Wire.write(0x03); 
  Wire.endTransmission();

  delay(500);
}

void loop() {
  
    Wire.beginTransmission(address);
    int error = Wire.endTransmission();
    if (error) {
    }else {

      Serial.println("Success");

    for (uint8_t reg = 0x94; reg < 0x9C; reg+=2) {
      Wire.beginTransmission(address);
      Wire.write(reg);
      Wire.endTransmission();

      Wire.requestFrom(address, 1);
      if(Wire.available()){
          LSByte = Wire.read();
      }

      Wire.beginTransmission(address);
      Wire.write(reg);
      Wire.endTransmission();

      Wire.requestFrom(address, 1);
      if(Wire.available()){
          MSByte = Wire.read();
      }

      regValue = (MSByte<<8) + LSByte;
      Serial.print(reg);
      Serial.print(" Register Value: ");
      Serial.println(regValue);
      // Wait 1 second before next reading
      delay(1000);
    }
    }
}