#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>

#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#define DHTPIN 16
#define DHTTYPE    DHT11     // DHT 11

WiFiMulti wifiMulti;
DHT_Unified dht(DHTPIN, DHTTYPE);

const char* ssid = "JAVON PIXEL 6"; //Your Wifi's SSID
const char* password = "PixelatedIndeed"; //Wifi Password

const char* laptopAt = "http://192.168.81.240:3237"; //change to your Laptop's IP

struct measurements {
  float temp;
  float humidity;
};

struct measurements getMeasurements() {
  struct measurements mmts;
  // Get temperature event and print its value.
  sensors_event_t event;
  dht.temperature().getEvent(&event);
  volatile float temperature = event.temperature;
  if (isnan(temperature)) {
    Serial.println(F("Error reading temperature!"));
  }
  else {
    mmts.temp = temperature;
  }
  // Get humidity event and print its value.
  dht.humidity().getEvent(&event);
  volatile float humidity = event.relative_humidity;
  if (isnan(humidity)) {
    Serial.println(F("Error reading humidity!"));
  }
  else {
    mmts.humidity = humidity;
  }
  return mmts;
}

void setup() {

    Serial.begin(115200);
    //Initialize DHT11.
    dht.begin();

    for(uint8_t t = 4; t > 0; t--) {
        Serial.printf("[SETUP] WAIT %d...\n", t);
        Serial.flush();
        delay(1000);
    }

    wifiMulti.addAP(ssid, password);

    while (wifiMulti.run() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
}

void loop() {
  if (WiFi.status() == WL_CONNECTED){
    Serial.println("Sending...");

    HTTPClient http;
    
    struct measurements mmts = getMeasurements();
    
    String url = laptopAt;
    // url += "/data?data1=31.2&data2=76"; //hardcoded values for example only
    url += "/data?data1={temp}&data2={humidity}";
  
    // Replace the placeholders with actual values
    url.replace("{temp}", String(mmts.temp));
    url.replace("{humidity}", String(mmts.humidity));
    
    http.begin(url);
    int returnCode = http.GET();   //perform a HTTP GET request
    
    if (returnCode > 0){
      String payload = http.getString();
      Serial.println(payload);
    }
    http.end();
    
  } else {
    Serial.println("WiFi disconnected");
  }
  delay(1000);  //Five second delay
}
