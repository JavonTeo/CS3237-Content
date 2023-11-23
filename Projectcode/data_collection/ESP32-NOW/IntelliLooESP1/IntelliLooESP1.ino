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

// WiFi Connection Libraries / AWS pub-sub Topics
#include "Secrets.h" // Customer file placed in libraries, to store AWS Certificates/Keys
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include <ArduinoJson.h> // For storing collected data into JSON format
#define AWS_IOT_PUBLISH_TOPIC   "esp32/pub"
#define AWS_IOT_SUBSCRIBE_TOPIC "esp32/sub"
WiFiClientSecure net = WiFiClientSecure();
PubSubClient client(net);

#include <esp_now.h> // For ESP-NOW communication between ESP32 devices

#include <time.h> // For getting Date and Time from NTP Server
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 28800; // 28800 seconds corresponds to 8 hours, as SGT is UTC+8.
const int   daylightOffset_sec = 0; // SG does not have daylight saving.

typedef struct datetime {
  char dateString[11];
  char timeString[6];
}datetime;

// Structure example to receive data
// Must match the sender structure
typedef struct esp2_struct_message {
  int id;
  float temperature;
  float humidity;
  int gas;
  int dampness;
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
  // Serial.print("Packet received from: ");
  // snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
  //          mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  // Serial.println(macStr);
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
  // Serial.println("BOARD 2:");
  // Serial.println(board2.id);
  // Serial.println(board2.temperature);
  // Serial.println(board2.humidity);
  // Serial.println(board2.gas);
  // Serial.println(board2.dampness);
  // Serial.println(board2.trash);
  // Serial.println("BOARD 3:");
  // Serial.println(board3.id);
  // Serial.println(board3.motion);
  // Serial.println(board3.rating);
  // Serial.println();
}

// Connects to AWS
void connectAWS()
{
  // WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
 
  Serial.println("Connecting to Wi-Fi");
 
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  // Init NTP
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
 
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
  datetime dt = getTime();
  float temperature = board2.temperature;
  float humidity = board2.humidity;
  int gas = board2.gas;
  int dampness = board2.dampness;
  float trash = board2.trash;

  int motion = board3.motion;
  float rating = board3.rating;

  // || isnan(motion) || isnan(rating)
  // if ((temperature == -1) || (humidity == -1) || (gas == -1) || (dampness == -1) || (trash == -1) || (dt.dateString == "0000-00-00") || (dt.timeString == "00:00"))
  // {
  //   Serial.println("Error: One or more sensor values are not available.");
  //   return; // Do not publish if any sensor value is missing
  // }

  StaticJsonDocument<200> doc;
  doc["day"] = dt.dateString;
  doc["time"] = dt.timeString;
  doc["temperature"] = temperature;
  doc["humidity"] = humidity;
  doc["gas"] = gas;
  doc["dampness"] = dampness;
  doc["trash"] = trash;
  doc["motion"] = motion;
  doc["rating"] = rating;
  char jsonBuffer[512];
  serializeJson(doc, jsonBuffer); // print to client
 
  bool sent = client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer);
  // Serial.print("SENT:");
  // Serial.println(sent);
  delay(1000); // Delay required for message to publish fully
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

// Get local time
struct datetime getTime(){
  datetime dt;
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    strcpy(dt.dateString, "0000-00-00");
    strcpy(dt.timeString, "00:00");
    return dt;
  }
  // Build date string (YYYY-MM-DD) // "YYYY-MM-DD\0" requires 11 characters
  sprintf(dt.dateString, "%04d-%02d-%02d", timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday);

  // Build time string (HH:mm) // "HH:mm\0" requires 6 characters
  sprintf(dt.timeString, "%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min);

  // Print the date and time strings
  Serial.print("Date: ");
  Serial.println(dt.dateString);
  Serial.print("Time: ");
  Serial.println(dt.timeString);

  return dt;
}
 
void setup() {
  //Initialize Serial Monitor
  Serial.begin(115200);
  //Set device as a Wi-Fi Station
  bool check = WiFi.mode(WIFI_STA);
  Serial.print("CHECK:");
  Serial.println(check);

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
    // Connects to AWS, publishes data to AWS, then disconnects WiFi.
    connectAWS();
    publishMessage();
    Serial.println("Sent to AWS!");
    board2_filled = false;
    board3_filled = false;
    ESP.restart();
  }
}
