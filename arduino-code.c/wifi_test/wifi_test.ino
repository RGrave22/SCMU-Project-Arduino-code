
#include <Arduino.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

// Replace these with your actual Firebase project credentials
#define WIFI_SSID "OpenFCT"
#define WIFI_PASSWORD ""
#define API_KEY "AIzaSyDB8cIkBU5lxgIqYjr7YKDWvtlyf5SIeQ4"
#define DATABASE_URL "https://scmu-7d9e7-default-rtdb.europe-west1.firebasedatabase.app/"  // e.g., https://your-project.firebaseio.com/

// Firebase objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
bool signupOK = false;

void setup() {
  Serial.begin(115200);

  // Connect to Wi-Fi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.println("Connected with IP: " + WiFi.localIP().toString());

  // Configure Firebase
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  // Anonymous signup
  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("Firebase signUp successful");
    signupOK = true;
  } else {
    Serial.printf("SignUp failed: %s\n", config.signer.signupError.message.c_str());
  }

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

void loop() {
  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 10000 || sendDataPrevMillis == 0)) {
    sendDataPrevMillis = millis();

    // Example: send values
    Firebase.RTDB.setInt(&fbdo, "Test/intValue", 123);
    Firebase.RTDB.setFloat(&fbdo, "Test/floatValue", 45.67);
    Firebase.RTDB.setString(&fbdo, "Test/message", "Hello Firebase!");

    // Check result
    if (fbdo.httpCode() == 200) {
      Serial.println("Data sent successfully:");
      Serial.println("Path: " + fbdo.dataPath());
      Serial.println("Type: " + fbdo.dataType());
      Serial.println("Value: " + fbdo.stringData());
    } else {
      Serial.println("Failed to send data: " + fbdo.errorReason());
    }
  }
}