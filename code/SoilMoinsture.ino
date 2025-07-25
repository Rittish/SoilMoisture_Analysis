#include <ESP8266WiFi.h>
#include <Firebase_ESP_Client.h>

// WiFi Configuration
#define WIFI_SSID "mithun"
#define WIFI_PASSWORD "mitnzz"

// Firebase Configuration
#define FIREBASE_HOST "cloud-ledcontro-default-rtdb.asia-southeast1.firebasedatabase.app/"
#define FIREBASE_AUTH "y1CGiJGdXo756WDdTa8T9ji8xFLnEfpS2DT2Zp"

// Pin Definitions
const int SOIL_PIN = A0;
const int LED_RED = D5;
const int LED_GREEN = D6;
const int LED_BLUE = D7;


const int DRY = 800;    // Dry soil threshold
const int WET = 300;     // Wet soil threshold

// Update interval (5 seconds)
const unsigned long UPDATE_INTERVAL = 5000; 

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

unsigned long previousMillis = 0;

void setup() {
  Serial.begin(115200);
  
  // Initialized LED pins
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  
  connectToWiFi();
  
  // Firebase configuration
  config.host = FIREBASE_HOST;
  config.signer.tokens.legacy_token = FIREBASE_AUTH;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

void loop() {
  unsigned long currentMillis = millis();
  
  if (currentMillis - previousMillis >= UPDATE_INTERVAL) {
    previousMillis = currentMillis;
    
    int moisture = analogRead(SOIL_PIN);
    
    String status;
    if(moisture > DRY) {
      status = "dry";
      digitalWrite(LED_RED, LOW);
      digitalWrite(LED_GREEN, HIGH);
      digitalWrite(LED_BLUE, LOW);
    } 
    else if(moisture > WET) {
      status = "normal";
      digitalWrite(LED_RED, LOW);
      digitalWrite(LED_GREEN, LOW);
      digitalWrite(LED_BLUE, HIGH);
    }
    else {
      status = "wet";
      digitalWrite(LED_RED, HIGH);
      digitalWrite(LED_GREEN, LOW);
      digitalWrite(LED_BLUE, LOW);
    }

    // Prints to serial
    Serial.print("Moisture: ");
    Serial.print(moisture);
    Serial.print(" - Status: ");
    Serial.println(status);

    // Sends to Firebase
    if(WiFi.status() == WL_CONNECTED) {
      Firebase.RTDB.setInt(&fbdo, "moisture/value", moisture);
      Firebase.RTDB.setString(&fbdo, "moisture/status", status);
      Firebase.RTDB.setInt(&fbdo, "moisture/timestamp", millis());
      
      if (Firebase.ready()) {
        String path = "history/" + String(millis());
        Firebase.RTDB.setInt(&fbdo, path + "/value", moisture);
        Firebase.RTDB.setString(&fbdo, path + "/status", status);
      }
    }
  }
}

void connectToWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}
