#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <ThingSpeak.h>

#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#define DHTPIN 16 // VCC: 5V / 3.3V
#define DHTTYPE DHT11
DHT_Unified dht(DHTPIN, DHTTYPE);

#define MQ2_AOPIN 36 // 36 is at SVP. VCC: 5V
#define RAIN_AOPIN 39 // 39 is at SVN. VCC: 3.3V
#define IR_DOPIN 14 // VCC: 3.3V

WiFiClient client;

const char* ssid = "SINGTEL-00CE";
const char* password = "cepahhifui";

unsigned long myChannelID = 2294439;
const char * myWriteAPIKey = "GALZQLFOOGRRQ2J3";

unsigned long lastTime = 0;
unsigned long timerDelay = 300000; //600000 ms = 10 min -> updates cloud every 10 minutes.
int humanCount = 0;
bool state = true;

struct dhtValues {
  float temperature;
  float humidity;
};

struct dhtValues read_dht() {
  struct dhtValues dhtVals;

  // Get temperature event.
  sensors_event_t event;
  dht.temperature().getEvent(&event);
  volatile float temperature = event.temperature;
  if (isnan(temperature)) {
    Serial.println(F("Error reading temperature!"));
  } else {
    dhtVals.temperature = temperature;
  }
  // Get humidity event.
  dht.humidity().getEvent(&event);
  volatile float humidity = event.relative_humidity;
  if (isnan(humidity)) {
    Serial.println(F("Error reading humidity!"));
  } else {
    dhtVals.humidity = humidity;
  }
  
  // Serial.print(F("Temperature: "));
  // Serial.print(temperature);
  // Serial.print(F("°C, "));
  // Serial.print(F("Humidity: "));
  // Serial.print(humidity);
  // Serial.println(F("%")); 
  return dhtVals;
}

int read_gas() {
  // MQ2 gas sensor to read ppm. Analog data. Test using a plastic spark wheel lighter, just press the button without sparking it.
  int gasValue = analogRead(MQ2_AOPIN);
  Serial.print("MQ2 sensor AO value: ");
  // Serial.println(gasValue);
  return gasValue;
}

int read_damp() {
  // Rain sensor to read dampness of toilet floor. Analog data.
  int dampness = analogRead(RAIN_AOPIN);
  Serial.print("Rain sensor AO value: ");
  // Serial.println(dampness);
  return dampness;
}

void read_depth() {
  // Infrared HC-SR04 sensor to read depth of trash bin. Analog data.

}

void read_motion() {  
  // HC-SR501 sensor to read human count into toilet in a 10 min interval. Digital data.
  if (!digitalRead(IR_DOPIN) && state){
    humanCount++;
    state = false;
    Serial.print("Human Count: ");
    Serial.println(humanCount);
    delay(100);
  }
  if (digitalRead(IR_DOPIN)) {
    state = true;
    delay(100);
  }
}

void setup() {
  Serial.begin(115200);
  dht.begin();
  pinMode(IR_DOPIN, INPUT);

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

    struct dhtValues dhtVals = read_dht();
    int gasValue = read_gas();
    int dampness = read_damp();
    
    ThingSpeak.setField(1, dhtVals.temperature);
    ThingSpeak.setField(2, dhtVals.humidity);
    ThingSpeak.setField(3, gasValue);
    ThingSpeak.setField(4, dampness);
    // ThingSpeak.setField(5, depth);
    ThingSpeak.setField(6, humanCount);
    ThingSpeak.writeFields(myChannelID, myWriteAPIKey);
        
    lastTime = millis();
    humanCount = 0;
  }

  // continuous data
  read_motion();
}
