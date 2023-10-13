#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

#include "Arduino.h"
#include <WiFi.h>
#include "ESP32MQTTClient.h"

#define DHTPIN 16
#define DHTTYPE    DHT11

const char *ssid = "JAVON PIXEL 6";
const char *pass = "PixelatedIndeed";

// Test Mosquitto server, see: https://test.mosquitto.org
char *server = "mqtt://192.168.113.55:1883"; //ip address should be your Windows Wi-Fi address

char *subscribeTopic = "hello/world";
char *publishTopic = "hello/esp";

char *publishTempTopic = "weather/temp";
char *publishHumidTopic = "weather/humidity";

ESP32MQTTClient mqttClient; // all params are set later

DHT_Unified dht(DHTPIN, DHTTYPE);
uint32_t delayMS;

struct measurements {
  float temperature;
  float humidity;
};

void setup()
{
    Serial.begin(115200);

    dht.begin();
    sensor_t sensor;
    delayMS = sensor.min_delay / 1000;

    log_i();
    log_i("setup, ESP.getSdkVersion(): ");
    log_i("%s", ESP.getSdkVersion());

    mqttClient.enableDebuggingMessages();

    mqttClient.setURI(server);
    mqttClient.enableLastWillMessage("lwt", "I am going offline");
    mqttClient.setKeepAlive(30);
    WiFi.begin(ssid, pass);
    WiFi.setHostname("c3test");
    mqttClient.loopStart();
}

int pubCount = 0;

void loop()
{
    struct measurements mmts = dht11_loop(); // this collects data from the DHT11 sensor. 

    String msg = "Hello: " + String(pubCount++);
    // mqttClient.publish(publishTopic, msg, 0, false);
    mqttClient.publish(publishTempTopic, String(mmts.temperature), 0, false);
    mqttClient.publish(publishHumidTopic, String(mmts.humidity), 0, false);
    delay(2000);
}

struct measurements dht11_loop() {
  // Delay between measurements.
  delay(delayMS);
  struct measurements mmts;
  // Get temperature event and print its value.
  sensors_event_t event;
  dht.temperature().getEvent(&event);
  if (isnan(event.temperature)) {
    Serial.println(F("Error reading temperature!"));
  }
  else {
    mmts.temperature = event.temperature;
    // Serial.print(F("Temperature: "));
    // Serial.print(event.temperature);
    // Serial.println(F("°C"));
  }
  // Get humidity event and print its value.
  dht.humidity().getEvent(&event);
  if (isnan(event.relative_humidity)) {
    Serial.println(F("Error reading humidity!"));
  }
  else {
    mmts.humidity = event.relative_humidity;
    // Serial.print(F("Humidity: "));
    // Serial.print(event.relative_humidity);
    // Serial.println(F("%"));
  }
  return mmts;
}

void onConnectionEstablishedCallback(esp_mqtt_client_handle_t client)
{
    if (mqttClient.isMyTurn(client)) // can be omitted if only one client
    {
        mqttClient.subscribe(subscribeTopic, [](const String &payload)
                             { log_i("%s: %s", subscribeTopic, payload.c_str());
                             Serial.print(payload.c_str()); 
                              });

        mqttClient.subscribe("bar/#", [](const String &topic, const String &payload)
                             { log_i("%s: %s", topic, payload.c_str()); });
    }
}

esp_err_t handleMQTT(esp_mqtt_event_handle_t event)
{
    mqttClient.onEventCallback(event);
    return ESP_OK;
}
