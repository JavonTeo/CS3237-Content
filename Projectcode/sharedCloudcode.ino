#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <ThingSpeak.h>

WiFiClient client;

const char* ssid = <your_wifi_name>; // make sure your wifi name is correct. If facing issues, run File > Examples > WiFi > WiFiScan. This will load a sketch that scans Wi-Fi networks within the range of your ESP32 board.
const char* password = <your_wifi_pw>;

unsigned long myChannelID = 2294895;
const char * myWriteAPIKey = "H1UKJIVRYBBBHVFC";

unsigned long lastTime = 0;
unsigned long timerDelay = 10000;

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
        Serial.println("Attempting to connect");
        while(WiFi.status() != WL_CONNECTED){
          Serial.println(WiFi.status()); //prints the wifi status code. For debugging.
          WiFi.begin(ssid, password); 
          delay(5000);     
        } 
        Serial.println("\nConnected.");
    }

    // The cloud's table has 6 columns, namely -
    // 1: Temperature, 2: Humidity, 3:  Gas concentration, 4: Floor dampness, 5: Trash bin depth, 6: Human count
    ThingSpeak.setField(1, temperature);
    ThingSpeak.setField(2, humidity);
    ThingSpeak.setField(3, gasConcentration);
    ThingSpeak.setField(4, dampness);
    ThingSpeak.setField(5, depth);
    ThingSpeak.setField(6, humanCount);
    ThingSpeak.writeFields(myChannelID, myWriteAPIKey); // this line writes the data you set (using setField) into the cloud
    
    lastTime = millis();
  }
}
