// WiFi Connection Libraries / AWS pub-sub Topics
#include "Secrets.h" // Customer file placed in libraries, to store AWS Certificates/Keys
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include "WiFi.h"
#include <ArduinoJson.h> // For storing collected data into JSON format
#define AWS_IOT_PUBLISH_TOPIC   "esp32/pub"
#define AWS_IOT_SUBSCRIBE_TOPIC "esp32/sub"
WiFiClientSecure net = WiFiClientSecure();
PubSubClient client(net);

// Sensor libraries / Pin numbers
#include <Adafruit_Sensor.h>
#include "DHT.h"
#define DHTPIN 16     // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11   // DHT 11
DHT dht(DHTPIN, DHTTYPE);
#define MQ2_AOPIN 36 // 36 is at SVP. VCC: 5V
#define RAIN_AOPIN 39 // 39 is at SVN. VCC: 3.3V
#define IR_DOPIN 14 // VCC: 3.3V

// Time values and HC-SR501 sensor global variables
unsigned long lastTime = 0;
unsigned long timerDelay = 30000; //600000 ms = 10 min -> updates cloud every 10 minutes.
int humanCount = 0;
bool state = true;

struct dhtValues {
  float temperature;
  float humidity;
};

struct dhtValues read_dht() {
  struct dhtValues dhtVals;

  float h = dht.readHumidity();
  float t = dht.readTemperature();
 
  if (isnan(h) || isnan(t) )  // Check if any reads failed and exit early (to try again).
  {
    Serial.println(F("Failed to read from DHT sensor!"));
    dhtVals.humidity = 0;
    dhtVals.temperature = 0;
    // return;
  }

  dhtVals.humidity = h;
  dhtVals.temperature = t;
 
  // Serial.print(F("Humidity: "));
  // Serial.print(h);
  // Serial.print(F("%  Temperature: "));
  // Serial.print(t);
  // Serial.println(F("°C "));

  return dhtVals;
}

int read_gas() {
  // MQ2 gas sensor to read ppm. Analog data. Test using a plastic spark wheel lighter, just press the button without sparking it.
  int gasValue = analogRead(MQ2_AOPIN);
  // Serial.print("MQ2 sensor AO value: ");
  // Serial.println(gasValue);
  return gasValue;
}

int read_damp() {
  // Rain sensor to read dampness of toilet floor. Analog data.
  int dampness = analogRead(RAIN_AOPIN);
  // Serial.print("Rain sensor AO value: ");
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
    // Serial.print("Human Count: ");
    // Serial.println(humanCount);
    delay(100);
  }
  if (digitalRead(IR_DOPIN)) {
    state = true;
    delay(100);
  }
}

// Connects to AWS
void connectAWS()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
 
  Serial.println("Connecting to Wi-Fi");
 
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
 
  // Configure WiFiClientSecure to use the AWS IoT device credentials
  net.setCACert(AWS_CERT_CA);
  net.setCertificate(AWS_CERT_CRT);
  net.setPrivateKey(AWS_CERT_PRIVATE);
 
  // Connect to the MQTT broker on the AWS endpoint we defined earlier
  client.setServer(AWS_IOT_ENDPOINT, 8883);
 
  // Create a message handler
  client.setCallback(messageHandler);
 
  Serial.println("Connecting to AWS IOT");
 
  while (!client.connect(THINGNAME))
  {
    Serial.print(".");
    delay(100);
  }
 
  if (!client.connected())
  {
    Serial.println("AWS IoT Timeout!");
    return;
  }
 
  // Subscribe to a topic
  client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC);
 
  Serial.println("AWS IoT Connected!");
}

// Publishes messages to the cloud at AWS_IOT_PUBLISH_TOPIC
void publishMessage()
{
  struct dhtValues dhtVals = read_dht();
  int gasValue = read_gas();
  int dampness = read_damp();

  StaticJsonDocument<200> doc;
  doc["humidity"] = dhtVals.humidity;
  doc["temperature"] = dhtVals.temperature;
  doc["gasValue"] = gasValue;
  doc["dampness"] = dampness;
  doc["humanCount"] = humanCount;
  char jsonBuffer[512];
  serializeJson(doc, jsonBuffer); // print to client
 
  client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer);
}

// Handles messages received from the cloud at AWS_IOT_SUBSCRIBE_TOPIC
void messageHandler(char* topic, byte* payload, unsigned int length)
{
  Serial.print("incoming: ");
  Serial.println(topic);
 
  StaticJsonDocument<200> doc;
  deserializeJson(doc, payload);
  const char* message = doc["message"];
  Serial.println(message);
}

void setup()
{
  Serial.begin(115200);
  dht.begin();
  pinMode(IR_DOPIN, INPUT);

  connectAWS();
}

void loop()
{
 if ((millis() - lastTime) > timerDelay) {
  publishMessage();

  lastTime = millis();
  humanCount = 0;
 }

 read_motion();
 client.loop();
}

