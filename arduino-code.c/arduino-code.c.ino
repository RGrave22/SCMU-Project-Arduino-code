#include <Arduino.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <PubSubClient.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

//Wifi credentials
#define WIFI_SSID "Grave"
#define WIFI_PASSWORD "grave123"

//Firebase credentials
#define API_KEY "AIzaSyDB8cIkBU5lxgIqYjr7YKDWvtlyf5SIeQ4"
#define DATABASE_URL "https://scmu-7d9e7-default-rtdb.europe-west1.firebasedatabase.app/" 

//MQTT credentias
#define MQTT_BROKER_ADRESS "broker.hivemq.com"
#define MQTT_CLIENT_NAME "botaniq_client"
#define MQTT_PORT 1883

/* VERIFY IF THE WORKS WITH THE DEFINES ABOVE, IF NOT DELETE THE DEFINES AND UNCOMMENT THIS
const char *MQTT_BROKER_ADRESS = "broker.hivemq.com";
const uint16_t MQTT_PORT = 1883;
const char *MQTT_CLIENT_NAME = "botaniq_client";
*/

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

unsigned long sendDataPrevMillis = 0;
bool signupOK = false;

void setup() 
{
  Serial.begin(115200);

  //Setups
  setupWifi();
  setupFirebase();
  setupMQTT();
}

void loop() 
{
  HandleMQTT();

  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 10000 || sendDataPrevMillis == 0)) {
    sendDataPrevMillis = millis();

    Firebase.RTDB.setInt(&fbdo, "Test/intValue", 123);
    Firebase.RTDB.setFloat(&fbdo, "Test/floatValue", 45.67);
    Firebase.RTDB.setString(&fbdo, "Test/message", "Hello Firebase!");
    
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


void setupWifi() 
{
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }

  Serial.println();
  Serial.println("Connected with IP: " + WiFi.localIP().toString());
}

void setupFirebase()
{
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("Firebase signUp successful");
    signupOK = true;
  } else {
    Serial.printf("SignUp failed: %s\n", config.signer.signupError.message.c_str());
  }

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}


void setupMQTT()
{
    mqttClient.setServer(MQTT_BROKER_ADRESS, MQTT_PORT);
    mqttClient.setCallback(OnMqttReceived);
}

void subscribeMQTT()
{ //ADD MORE WHEN NEEDED
    mqttClient.subscribe("cocomole");
}

void OnMqttReceived(char *topic, byte *payload, unsigned int length)
{
    Serial.print("Received on ");
    Serial.print(topic);
    Serial.print(": ");

    String content = "";
    for (size_t i = 0; i < length; i++)
    {
        content.concat((char)payload[i]);
    }
    Serial.print(content);
    Serial.println();
}

void HandleMQTT()
{
    if (!mqttClient.connected())
    {
        ConnectMqtt();
    }
    mqttClient.loop();
}

void ConnectMQTT()
{
    Serial.print("Starting MQTT connection...");
    if (mqttClient.connect(MQTT_CLIENT_NAME))
    {
        SuscribeMqtt();
        mqttClient.publish("connected","hello/world");
    }
    else
    {
        Serial.print("Failed MQTT connection, rc=");
        Serial.print(mqttClient.state());
        Serial.println(" try again in 5 seconds");

        delay(5000);
    }
}

void readMacAddress()
{
    WiFi.mode(WIFI_STA);
    WiFi.STA.begin();
    Serial.print("[DEFAULT] ESP32 Board MAC Address: ");
    uint8_t baseMac[6];
    esp_err_t ret = esp_wifi_get_mac(WIFI_IF_STA, baseMac);
    if (ret == ESP_OK)
    {
        Serial.printf("%02x:%02x:%02x:%02x:%02x:%02x\n",
                      baseMac[0], baseMac[1], baseMac[2],
                      baseMac[3], baseMac[4], baseMac[5]);
    }
    else
    {
        Serial.println("Failed to read MAC address");
    }
    char s[20];
    sprintf(s, "%02x:%02x:%02x:%02x:%02x:%02x\n",
            baseMac[0], baseMac[1], baseMac[2],
            baseMac[3], baseMac[4], baseMac[5]);

    u8g2.begin();
    u8g2.clearDisplay();
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB08_tr);
    u8g2.drawStr(3, 10, s);
    u8g2.sendBuffer();
}





