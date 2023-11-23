#include <esp_now.h>
#include <WiFi.h>

// Sensor libraries / Pin numbers
#include <Adafruit_Sensor.h>
  // Temperature Sensor Setup
#include "DHT.h"
#define DHTPIN 16     // Digital pin connected to the DHT sensor
// #define DHTTYPE DHT21   // DHT 21 sensor
#define DHTTYPE DHT11   // DHT 11 sensor
DHT dht(DHTPIN, DHTTYPE);
  // MQ2 Sensor Setup
#define MQ2_AOPIN 36 // 36 is at SVP. VCC: 5V
  // Rain Sensor Setup
#define RAIN_AOPIN 39 // 39 is at SVN. VCC: 3.3V
  // Ultrasound Sensor Setup
#define HC_TRIGPIN 5 
#define HC_ECHOPIN 18

// Deep Sleep
#define uS_TO_S_FACTOR 1000000ULL
#define TIME_TO_SLEEP 10

// REPLACE WITH THE RECEIVER'S MAC Address
< uint8_t broadcastAddress[] = {0xXX, 0xXX, 0xXX, 0xXX, 0xXX, 0xXX}; >

// Structure example to send data
// Must match the receiver structure
typedef struct esp2_struct_message {
  int id;
  float temperature;
  float humidity;
  int gas;
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
    dhtVals.humidity = -1;
    dhtVals.temperature = -1;
    return dhtVals;
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
  if (gasValue < 0 || gasValue > 4095) {
    Serial.println("Error: Invalid dampness reading.");
    return -1; // or any other value that signifies an error
  }
  // Serial.print("MQ2 sensor AO value: ");
  // Serial.println(gasValue);
  return gasValue;
}

int read_damp() {
  // Rain sensor to read dampness of toilet floor. Analog data.
  int dampness = analogRead(RAIN_AOPIN);
  if (dampness < 0 || dampness > 4096) {
    Serial.println("Error: Invalid dampness reading.");
    return -1; // or any other value that signifies an error
  }
  // Serial.print("Rain sensor AO value: ");
  // Serial.println(dampness);
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
  if (isnan(distanceCm)) {
    return -1;
  }
  
  // Convert to inches
  // float distanceInch = distanceCm * CM_TO_INCH;
  if (distanceCm > 15.00) {
    distanceCm = 15.00;
  }
  
  // Prints the distance in the Serial Monitor
  // Serial.print("Distance (cm): ");
  // Serial.println(distanceCm);
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

  dht.begin();
  pinMode(HC_TRIGPIN, OUTPUT);
  pinMode(HC_ECHOPIN, INPUT);
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
  Serial.println(myData.id);
  Serial.println(myData.temperature);
  Serial.println(myData.humidity);
  Serial.println(myData.gas);
  Serial.println(myData.dampness);
  Serial.println(myData.trash);

  // Send message via ESP-NOW
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
   
  if (result == ESP_OK) {
    Serial.println("Sent with success");
  }
  else {
    Serial.println("Error sending the data");
  }

  delay(100); // buffer time for successfully sending ESP-NOW
  Serial.println("Going to sleep now");
  Serial.flush(); 
  esp_deep_sleep_start();
}
