#include <esp_now.h>
#include <WiFi.h>

const int buttonPin1 = 18; // Replace with the actual pin numbers for your buttons
const int buttonPin2 = 5;
const int buttonPin3 = 16;
const int buttonPin4 = 4;
const int buttonPin5 = 2;   

volatile bool button1Pressed = false;
volatile bool button2Pressed = false;
volatile bool button3Pressed = false;
volatile bool button4Pressed = false;
volatile bool button5Pressed = false;

volatile int pressCount = 0;
volatile int lastDebounceTime = 0;
volatile int debounceDelay = 200;

const int maxSize = 50; // Define the maximum size of the array
int myArray[maxSize];   // Create an integer array
int currentIndex = 0;   // Initialize the index to 0

// PIR Sensor Setup
const int pirPin = 17;  // PIR sensor connected to digital pin 2
volatile int peopleCount = 0; // Initialize the people count variable

void appendNumber(int number) {
  if (currentIndex < maxSize) {
    myArray[currentIndex] = number; // Append the number to the array
    currentIndex++; // Increment the index
  }
}

float calculateAverage() {
  int sum = 0;
  for (int i = 0; i < currentIndex; i++) {
    sum += myArray[i];
  }
  if (currentIndex > 0) {
    return static_cast<float>(sum) / currentIndex;
  } else {
    return 0.0; // Return 0 if the array is empty to avoid division by zero
  }
}

void setup_peripherals() {
  Serial.begin(115200);

  pinMode(buttonPin1, INPUT_PULLDOWN);
  pinMode(buttonPin2, INPUT_PULLDOWN);
  pinMode(buttonPin3, INPUT_PULLDOWN);
  pinMode(buttonPin4, INPUT_PULLDOWN);
  pinMode(buttonPin5, INPUT_PULLDOWN);

  attachInterrupt(digitalPinToInterrupt(buttonPin1), button1ISR, RISING);
  attachInterrupt(digitalPinToInterrupt(buttonPin2), button2ISR, RISING);
  attachInterrupt(digitalPinToInterrupt(buttonPin3), button3ISR, RISING);
  attachInterrupt(digitalPinToInterrupt(buttonPin4), button4ISR, RISING);
  attachInterrupt(digitalPinToInterrupt(buttonPin5), button5ISR, RISING);
  
  pinMode(pirPin, INPUT);  // Set PIR pin as an input
  attachInterrupt(digitalPinToInterrupt(pirPin), motionDetected, RISING); // Attach an interrupt to the PIR pin when it goes HIGH (RISING edge)
}

void loop_peripherals() {
  if (button1Pressed) {
    Serial.println("Button 1 pressed.");
    appendNumber(1);
    pressCount++;
    button1Pressed = false; // Reset the flag
    
  }

  if (button2Pressed) {
    Serial.println("Button 2 pressed.");
    appendNumber(2);
    pressCount++;
    button2Pressed = false; // Reset the flag
  }

  if (button3Pressed) {
    Serial.println("Button 3 pressed.");
    appendNumber(3);
    pressCount++;
    button3Pressed = false; // Reset the flag
  }

  if (button4Pressed) {
    Serial.println("Button 4 pressed.");
    appendNumber(4);
    pressCount++;
    button4Pressed = false; // Reset the flag
  }

  if (button5Pressed) {
    Serial.println("Button 5 pressed.");
    appendNumber(5);
    pressCount++;
    button5Pressed = false; // Reset the flag
  }

  //delay(5000);
  //Serial.println(calculateAverage());
}

void motionDetected() {
  // This function is called when motion is detected by the PIR sensor
  peopleCount++; // Increment the people count
  Serial.print("Motion detected! People Count: ");
  Serial.println(peopleCount);
}

void button1ISR() {
  if(pressCount == 0 || (millis() - lastDebounceTime) > debounceDelay){
  button1Pressed = true;
  lastDebounceTime = millis();
  }
}

void button2ISR() {
  if(pressCount == 0 || (millis() - lastDebounceTime) > debounceDelay){
  button2Pressed = true;
  lastDebounceTime = millis();
  }
}

void button3ISR() {
  if(pressCount == 0 || (millis() - lastDebounceTime) > debounceDelay){
  button3Pressed = true;
  lastDebounceTime = millis();
  }
}

void button4ISR() {
  if(pressCount == 0 || (millis() - lastDebounceTime) > debounceDelay){
  button4Pressed = true;
  lastDebounceTime = millis();
  }
}

void button5ISR() {
  if(pressCount == 0 || (millis() - lastDebounceTime) > debounceDelay){
  button5Pressed = true;
  lastDebounceTime = millis();
  }
}

// REPLACE WITH THE RECEIVER'S MAC Address
< uint8_t broadcastAddress[] = {0xXX, 0xXX, 0xXX, 0xXX, 0xXX, 0xXX}; >
// uint8_t broadcastAddress[] = {0x64, 0xB7, 0x08, 0x60, 0xC6, 0x5C}; // Javon's ESP32 MAC

// Structure example to send data
// Must match the receiver structure
typedef struct esp3_struct_message {
  int id;
  int motion;
  float rating;
}esp3_struct_message;

// Create a struct_message called myData
esp3_struct_message myData;

// Create peer interface
esp_now_peer_info_t peerInfo;

// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}
 
void setup() {
  // Init Serial Monitor
  Serial.begin(115200);

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

  setup_peripherals();
}
 
void loop() {
  loop_peripherals();
  float rating = calculateAverage();
  // Set values to send
  myData.id = 1;
  myData.motion = peopleCount;
  myData.rating = rating;

  // Need to send every 10minutes
  // Send message via ESP-NOW
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
   
  if (result == ESP_OK) {
    Serial.println("Sent with success");
  }
  else {
    Serial.println("Error sending the data");
  }
  delay(10000);
}
