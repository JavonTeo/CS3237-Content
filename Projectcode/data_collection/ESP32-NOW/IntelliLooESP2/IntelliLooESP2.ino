#include <esp_now.h>
#include <WiFi.h>

// Sensor libraries / Pin numbers
#include <Adafruit_Sensor.h>
  // Temperature Sensor Setup
#include "DHT.h"
#define DHTPIN 16     // Digital pin connected to the DHT sensor
#define DHTTYPE DHT21   // DHT 21 sensor
// #define DHTTYPE DHT11   // DHT 11 sensor
DHT dht(DHTPIN, DHTTYPE);
  // MQ2 Sensor Setup
#define MQ2_AOPIN 15 
  // Rain Sensor Setup
#define RAIN_AOPIN 4 
  // Ultrasound Sensor Setup
#define HC_TRIGPIN 5 
#define HC_ECHOPIN 18

// Deep Sleep
#define uS_TO_S_FACTOR 1000000ULL
#define TIME_TO_SLEEP 300

// REPLACE WITH THE RECEIVER'S MAC Address
< uint8_t broadcastAddress[] = {0xXX, 0xXX, 0xXX, 0xXX, 0xXX, 0xXX}; >
// uint8_t broadcastAddress[] = {0x64, 0xB7, 0x08, 0x60, 0xC6, 0x5C}; // Javon's ESP32 MAC

// Structure example to send data
// Must match the receiver structure
typedef struct esp2_struct_message {
  int id;
  float temperature;
  float humidity;
  float gas;
  int dampness;
  float trash;
}esp2_struct_message;
// Create a struct_message called myData
esp2_struct_message myData;

// Create peer interface
esp_now_peer_info_t peerInfo;

// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

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
   Serial.print("MQ2 sensor AO value: ");
   Serial.println(gasValue);
  return gasValue;
}

int read_damp() {
  // Rain sensor to read dampness of toilet floor. Analog data.
  int dampness = analogRead(RAIN_AOPIN);
   Serial.print("Rain sensor AO value: ");
   Serial.println(dampness);
  return dampness;
}

float read_trash() {
  // Infrared HC-SR04 sensor to read depth of trash bin. Analog data.
  // Clears the trigPin
  float SOUND_SPEED  = 0.034;
  float CM_TO_INCH = 0.393701;
  digitalWrite(HC_TRIGPIN, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(HC_TRIGPIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(HC_TRIGPIN, LOW);

    // Reads the echoPin, returns the sound wave travel time in microseconds
  float duration = pulseIn(HC_ECHOPIN, HIGH);
  
  // Calculate the distance
  float distanceCm = duration * SOUND_SPEED/2;
  
  // Convert to inches
  // float distanceInch = distanceCm * CM_TO_INCH;
  if (distanceCm > 15.00) {
    distanceCm = 15.00;
  }
  
  // Prints the distance in the Serial Monitor
  Serial.print("Distance (cm): ");
  Serial.println(distanceCm);
  // Serial.print("Distance (inch): ");
  // Serial.println(distanceInch);
  
  delay(1000);
  return distanceCm;
}

void setup() {
  // Init Serial Monitor
  Serial.begin(115200);
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);
  
  // Register peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
}
 
void loop() {
  // Collect sensor data
  struct dhtValues dhtVals = read_dht();
  int gas = read_gas();
  int dampness = read_damp();
  float trash = read_trash();

  // Set values to send
  myData.id = 2;
  myData.temperature = dhtVals.temperature;
  myData.humidity = dhtVals.humidity;
  myData.gas = gas;
  myData.dampness = dampness;
  myData.trash = trash;

  // Send message via ESP-NOW
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
   
  if (result == ESP_OK) {
    Serial.println("Sent with success");
  }
  else {
    Serial.println("Error sending the data");
  }
  delay(5000);
  esp_deep_sleep_start();
}
