#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <ThingSpeak.h>

#define DHTPIN 16
#define DHTTYPE DHT11

const char* ssid = "SINGTEL-00CE";
const char* password = "cepahhifui";

DHT_Unified dht(DHTPIN, DHTTYPE);
WiFiClient client;

unsigned long myChannelID = 2294439;
const char * myWriteAPIKey = "GALZQLFOOGRRQ2J3";

unsigned long lastTime = 0;
unsigned long timerDelay = 5000;

void setup() {
  Serial.begin(115200);
  dht.begin();

  WiFi.mode(WIFI_STA);
  ThingSpeak.begin(client);
}

void loop() {
  if ((millis() - lastTime) > timerDelay) {
    // Connect or reconnect to WiFi
    if(WiFi.status() != WL_CONNECTED){
        Serial.print("Attempting to connect");
        while(WiFi.status() != WL_CONNECTED){
          Serial.println(WiFi.status());
          WiFi.begin(ssid, password); 
          delay(5000);     
        } 
        Serial.println("\nConnected.");
    }

    // Get temperature event.
    sensors_event_t event;
    dht.temperature().getEvent(&event);
    volatile float temperature = event.temperature;
    if (isnan(temperature)) {
      Serial.println(F("Error reading temperature!"));
    }
    // Get humidity event.
    dht.humidity().getEvent(&event);
    volatile float humidity = event.relative_humidity;
    if (isnan(humidity)) {
      Serial.println(F("Error reading humidity!"));
    }
    Serial.print(F("Temperature: "));
    Serial.print(temperature);
    Serial.print(F("°C, "));
    Serial.print(F("Humidity: "));
    Serial.print(humidity);
    Serial.println(F("%"));

    ThingSpeak.setField(1, temperature);
    ThingSpeak.setField(2, humidity);
    ThingSpeak.writeFields(myChannelID, myWriteAPIKey);
  }
}
