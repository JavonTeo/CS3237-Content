#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <ESP32Servo.h>

#include "Arduino.h"
#include <WiFi.h>
#include "ESP32MQTTClient.h"

#define uS_TO_S_FACTOR 1000000ULL  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  20        /* Time ESP32 will go to sleep (in seconds) */

#define DHTPIN 16
#define DHTTYPE    DHT11

const char *ssid = "JAVON PIXEL 6";
const char *pass = "PixelatedIndeed";

// Test Mosquitto server, see: https://test.mosquitto.org
char *server = "mqtt://192.168.113.240:1883"; //ip address should be your Windows Wi-Fi address

char *subscribeTempTopic = "classification/temp";
char *publishTempTopic = "weather/temp";
char *publishHumidTopic = "weather/humidity";

ESP32MQTTClient mqttClient; // all params are set later

DHT_Unified dht(DHTPIN, DHTTYPE);
uint32_t delayMS;

String classification = "";
Servo myservo;
int pos = 0;
int servoPin = 13;

struct measurements {
  float temperature;
  float humidity;
};

void setup()
{
    Serial.begin(115200);

    esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);

    dht.begin();
    sensor_t sensor;
    delayMS = sensor.min_delay / 1000;

    setup_servo();

    log_i();
    log_i("setup, ESP.getSdkVersion(): ");
    log_i("%s", ESP.getSdkVersion());

    mqttClient.enableDebuggingMessages();

    mqttClient.setURI(server);
    mqttClient.enableLastWillMessage("lwt", "I am going offline");
    mqttClient.setKeepAlive(30);
    WiFi.begin(ssid, pass);
    WiFi.setHostname("c3test");
    while(WiFi.status() != WL_CONNECTED) {
      Serial.print('.');
      delay(1000);
    }
    Serial.println("CONNECTED");
    mqttClient.loopStart();
}

void setup_servo()
{
  // Allow allocation of all timers
	ESP32PWM::allocateTimer(0);
	ESP32PWM::allocateTimer(1);
	ESP32PWM::allocateTimer(2);
	ESP32PWM::allocateTimer(3);
	myservo.setPeriodHertz(50);    // standard 50 hz servo
	myservo.attach(servoPin, 1000, 2000); // attaches the servo on pin 13 to the servo object
}

void loop()
{
  struct measurements mmts = dht11_loop(); // this collects data from the DHT11 sensor. 
  mqttClient.publish(publishTempTopic, String(mmts.temperature), 0, false);
  mqttClient.publish(publishHumidTopic, String(mmts.humidity), 0, false);
  delay(5000);
  actuateServo(classification);
  Serial.println("Going to sleep now");
  delay(5000);
  esp_deep_sleep_start(); // This makes ESP32 go to sleep.
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

void actuateServo(String reply) {
  if (reply == "HOT") {
    if (pos != 0) {
      myservo.write(0);
      pos = 0;
    }
  } else if (reply == "COLD") {
    if (pos != 180) {
      myservo.write(180);
      pos = 180;
    }
  } else if (reply == "BETWEEN") {
    if (pos != 90) {
      myservo.write(90);
      pos = 90;
    }
  } else {

  }
}

void onConnectionEstablishedCallback(esp_mqtt_client_handle_t client)
{
    if (mqttClient.isMyTurn(client)) // can be omitted if only one client
    {
        mqttClient.subscribe(subscribeTempTopic, [](const String &payload)
                             {
                              Serial.println(String(subscribeTempTopic)+String("")+String(payload.c_str()));
                              classification = String(payload.c_str());
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
