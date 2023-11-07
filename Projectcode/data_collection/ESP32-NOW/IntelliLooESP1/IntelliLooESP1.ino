/*********
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp-now-many-to-one-esp32/
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*********/

/**
How it works:
1. ESP32-2 sends data, which is stored in board2. board2_filled is set to true.
2. ESP32-3 sends data, which is stored in board3. board3_filled is set to true.
3. loop() checks if both board2_filled and board3_filled is true. If so, it continues to next step. Else, it just waits till this happens.
4. JSON containing the sensor data is constructed, then sent to AWS.
5. Both board2_filled and board3_filled is set to false.
6. Cycle repeats.
**/

#include <esp_now.h>

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

// Structure example to receive data
// Must match the sender structure
typedef struct esp2_struct_message {
  int id;
  float temperature;
  float humidity;
  float gas;
  int dampness,
  float trash;
}esp2_struct_message;

typedef struct esp3_struct_message {
  int id;
  int motion;
  float rating;
}esp3_struct_message;

// Create a structure to hold the readings from each board
esp2_struct_message board2;
esp3_struct_message board3;
bool board2_filled = false;
bool board3_filled = false;

// callback function that will be executed when data is received
void OnDataRecv(const uint8_t * mac_addr, const uint8_t *incomingData, int len) {
  char macStr[18];
  Serial.print("Packet received from: ");
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.println(macStr);
  // Update the structures with the new incoming data
  if (len == sizeof(board2)) {
    memcpy(&board2, incomingData, sizeof(board2));
    Serial.printf("Board ID %u: %u bytes\n", board2.id, len);
    board2_filled = true;
  } else if (len == sizeof(board3)) {
    memcpy(&board3, incomingData, sizeof(board3));
    Serial.printf("Board ID %u: %u bytes\n", board3.id, len);
    board3_filled = true;
  } else {
    Serial.println("Unknown data type");
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
  float temperature = board2.temperature;
  float humidity = board2.humidity;
  float gas = board2.gas;
  int dampness = board2.dampness,
  float trash = board2.trash;

  int motion = board3.motion;
  float rating = board3.rating;

  StaticJsonDocument<200> doc;
  doc["temperature"] = temperature;
  doc["humidity"] = humidity;
  doc["gas"] = gas;
  doc["dampness"] = dampness;
  doc["trash"] = trash;
  doc["motion"] = motion;
  doc["rating"] = rating;
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
 
void setup() {
  //Initialize Serial Monitor
  Serial.begin(115200);
  
  //Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);
  connectAWS();

  //Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  
  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info
  esp_now_register_recv_cb(OnDataRecv);
}
 
void loop() {
  if (board2_filled && board3_filled) {
    //do something
    publishMessage();
    board2_filled = false;
    board3_filled = false;
  }
}