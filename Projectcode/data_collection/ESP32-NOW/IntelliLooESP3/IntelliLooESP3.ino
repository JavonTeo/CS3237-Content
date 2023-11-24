/**
Collects average user rating and user count.
**/
#include <esp_now.h>
#include <WiFi.h>

// RTC
#include <Wire.h>
#include <RTClib.h>

RTC_DS3231 rtc;
RTC_DATA_ATTR DateTime sleepTime;

const int buttonPin1 = 34; // Replace with the actual pin numbers for your buttons
const int buttonPin2 = 32;
const int buttonPin3 = 33;
const int buttonPin4 = 25;
const int buttonPin5 = 26;
const int pirPin     = 35;   

volatile bool button1Pressed = false;
volatile bool button2Pressed = false;
volatile bool button3Pressed = false;
volatile bool button4Pressed = false;
volatile bool button5Pressed = false;

RTC_DATA_ATTR int pressCount = 0;
RTC_DATA_ATTR int rtcSetupOnce = 0;
volatile int lastDebounceTime = 0;
volatile int debounceDelay = 200;

RTC_DATA_ATTR int motionCount = 0;
RTC_DATA_ATTR int peopleCount = 0; // Initialize the people count variable
volatile bool motionFlag = false;

RTC_DATA_ATTR int totalRating = 0;

float calculateAverage(int totalRating, int pressCount) {
  return (pressCount != 0) ? (float)totalRating / pressCount : 0.0;
}

// DEEP SLEEP
#define uS_TO_S_FACTOR 1000000ULL  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  300        /* Time ESP32 will go to sleep (in seconds) */

void print_wakeup_reason(){
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch(wakeup_reason)
  {
    case ESP_SLEEP_WAKEUP_EXT0 : Serial.println("Wakeup caused by external signal using RTC_IO"); break;
    case ESP_SLEEP_WAKEUP_EXT1 : Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER : Serial.println("Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : Serial.println("Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP : Serial.println("Wakeup caused by ULP program"); break;
    default : Serial.printf("Wakeup was not caused by deep sleep: %d\n",wakeup_reason); break;
  }
}

void print_GPIO_wake_up(){
  uint64_t GPIO_reason = esp_sleep_get_ext1_wakeup_status();
  float GPIO_triggered = (log(GPIO_reason))/log(2);
  Serial.print("GPIO that triggered the wake up: GPIO ");
  Serial.println(GPIO_triggered, 0);
}

void buttonPressed(int GPIO) {
  switch (GPIO) {
    case buttonPin1:
      totalRating += 1;
      pressCount++;
      break;

    case buttonPin2:
      totalRating += 2;
      pressCount++;
      break;

    case buttonPin3:
      totalRating += 3;
      pressCount++;
      break;

    case buttonPin4:
      totalRating += 4;
      pressCount++;
      break;

    case buttonPin5:
      totalRating += 5;
      pressCount++;
      break;
    case pirPin:
      motionCount++;
      if (motionCount == 2) {
        peopleCount++;
        motionCount = 0;
      }

    default:
      // Handle the case when GPIO doesn't match any buttonPin
      break;
  }
}

// Determine bitmask: 2^25+2^26+2^32+2^33+2^34+2^35 = 64525172736 = 0xF06000000
#define BUTTON_PIN_BITMASK 0xF06000000

// REPLACE WITH THE RECEIVER'S MAC Address
uint8_t broadcastAddress[] = {0x40, 0x22, 0xD8, 0xF1, 0x4E, 0xD0};

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

void setup_peripherals() {
  Serial.begin(115200);

  pinMode(buttonPin1, INPUT_PULLDOWN);
  pinMode(buttonPin2, INPUT_PULLDOWN);
  pinMode(buttonPin3, INPUT_PULLDOWN);
  pinMode(buttonPin4, INPUT_PULLDOWN);
  pinMode(buttonPin5, INPUT_PULLDOWN);
  
  pinMode(pirPin, INPUT_PULLUP);  // Set PIR pin as an input
}

void RTCSetup() {
    if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }
  
  if (rtc.lostPower()) {
    Serial.println("RTC lost power, let's set the time!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  sleepTime = rtc.now();
  rtcSetupOnce = 1;
}

void ESPNowSetup() {
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

void setup() {
  // Init Serial Monitor
  Serial.begin(115200);

  // RTC Setup
  if (rtcSetupOnce = 0) {
    RTCSetup();
  }

  // ESPNow Setup
  ESPNowSetup();

  // Peripherals Setup
  setup_peripherals();

  //Print the wakeup reason for ESP32
  print_wakeup_reason();
  print_GPIO_wake_up();


  onWakeUp();
}

void onWakeUp() {
  DateTime wakeTime = rtc.now();
  Serial.println("Woke up at: " + String(wakeTime.hour()) + ":" + String(wakeTime.minute()) + ":" + String(wakeTime.second()));

  // Calculate time elapsed
  TimeSpan elapsedTime = wakeTime - sleepTime;
  int elapsedSeconds = elapsedTime.seconds();
  Serial.println("Time elapsed: " + String(elapsedTime.hours()) + "h " + String(elapsedTime.minutes()) + "m " + String(elapsedTime.seconds()) + "s");

  int goToSleep = TIME_TO_SLEEP - elapsedSeconds;
  if (goToSleep <= 0 || goToSleep >= TIME_TO_SLEEP){
    goToSleep = TIME_TO_SLEEP;
  }
  
  // Deep Sleep
  esp_sleep_enable_timer_wakeup(goToSleep * uS_TO_S_FACTOR);
  Serial.println("Setup ESP32 to sleep for every " + String(TIME_TO_SLEEP) + " Seconds");

  esp_sleep_wakeup_cause_t wakeup_reason;
  wakeup_reason = esp_sleep_get_wakeup_cause();

  if (wakeup_reason == ESP_SLEEP_WAKEUP_EXT1) {
    uint64_t GPIO_reason = esp_sleep_get_ext1_wakeup_status();
    float GPIO_triggered = (log(GPIO_reason))/log(2);
    buttonPressed(int(GPIO_triggered));
  }

  else if (wakeup_reason == ESP_SLEEP_WAKEUP_TIMER) {
    float rating = calculateAverage(totalRating, pressCount);
    
    // Set values to send
    myData.id = 3;
    myData.motion = peopleCount;
    myData.rating = rating;

    // Need to send every 10 minutes
    // Send message via ESP-NOW
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *)&myData, sizeof(myData));
    
    if (result == ESP_OK) {
      Serial.println("Sent with success");
    } else {
      Serial.println("Error sending the data");
    }
    pressCount = 0;
    totalRating = 0;
    peopleCount = 0;
    // Update sleepTime for the next sleep cycle
    sleepTime = rtc.now();
  }

  // We send ESP32 to sleep.
  Serial.println("Going to sleep now");
  delay(100);
  esp_deep_sleep_start(); // This makes ESP32 go to sleep.
}
 
void loop() {

}