#include <Arduino.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <PubSubClient.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"
#include <U8g2lib.h>
#include <esp_wifi.h>
#include <Wire.h>
#include "DHT.h"

//Wifi credentials
#define WIFI_SSID "Grave"
#define WIFI_PASSWORD "grave123"

//Firebase credentials
#define API_KEY "AIzaSyDB8cIkBU5lxgIqYjr7YKDWvtlyf5SIeQ4"
#define DATABASE_URL "https://scmu-7d9e7-default-rtdb.europe-west1.firebasedatabase.app/" 
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

//MQTT credentias
#define MQTT_BROKER_ADRESS "broker.hivemq.com"
#define MQTT_CLIENT_NAME "botaniq_client"
#define MQTT_PORT 1883
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

//Monitor
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

//Pins
#define BUTTONPIN 4
#define DHTPIN 18

//Button
int buttonNew = 0;

//DHT
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

void setup() 
{
  Serial.begin(9600);

  //Setups
  setupWifi();
  setupFirebase();
  setupMQTT();
  setupMonitor();
  setupButton();
  const char* mac = getMacAddress();
  Serial.println(mac);
}

void loop() 
{
  handleMQTT();
  handleButton();//responsable to Display the values on the screen


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
    //signupOK = true;
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

void setupMonitor()
{
  u8g2.begin();
  u8g2.clearDisplay();
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x10_tr);
  u8g2.drawStr(3, 10, "BotaniQ starting...");
  u8g2.sendBuffer();
}

void setupButton()
{
  pinMode(BUTTONPIN, INPUT_PULLUP);
}

void handleButton()
{
  buttonNew=digitalRead(BUTTONPIN);

  if(buttonNew == LOW){
    Serial.println("Showing MAC:");
    displayMacAddress();
    delay(3000);
  }else{
    showSensorValues();
    delay(3000);
  }
}

void subscribeMQTT()
{ 
    const char* mac = getMacAddress();  
    String topic = "greenhouse/";
    topic += mac;  

    //To remove the excess spaces
    topic.replace(":", "");

    mqttClient.subscribe(topic.c_str());  

    Serial.print("Subscribed to topic: ");
    Serial.println(topic);
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

void handleMQTT()
{
    if (!mqttClient.connected())
    {
        ConnectMQTT();
    }
    mqttClient.loop();
}

void ConnectMQTT()
{
    Serial.println("Starting MQTT connection...");
    if (mqttClient.connect(MQTT_CLIENT_NAME))
    {
        subscribeMQTT();
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

void displayMacAddress()
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
    sprintf(s, "%02x:%02x:%02x:%02x:%02x:%02x",
            baseMac[0], baseMac[1], baseMac[2],
            baseMac[3], baseMac[4], baseMac[5]);

    u8g2.begin();
    u8g2.clearDisplay();
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x10_tr); 
    u8g2.drawStr(0, 10, "Connection code:");
    u8g2.drawStr(0, 25, s);  
    u8g2.sendBuffer();
}


const char* getMacAddress() 
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
    static char s[20];
    sprintf(s, "%02x:%02x:%02x:%02x:%02x:%02x\n",
            baseMac[0], baseMac[1], baseMac[2],
            baseMac[3], baseMac[4], baseMac[5]);

    return s;
}

//To display all sensor values
void showSensorValues() {
  float temp = dht.readTemperature(); 
  float hum = dht.readHumidity();

  u8g2.setFontPosCenter();
  u8g2.clearBuffer();
  //u8g2.setFont(u8g2_font_10x20_te);
  u8g2.setFont(u8g2_font_6x10_tr);

  //Temos de por os valores de acordo com os sensores
  char temperature[10];
  dtostrf(temp, 6, 2, temperature);
  char humidity[10];
  dtostrf(hum, 6, 2, humidity);

  char tempDisplay[20];
  snprintf(tempDisplay, sizeof(tempDisplay), "TEMP: %s C", temperature);

  char humDisplay[20];
  snprintf(humDisplay, sizeof(humDisplay), "HUM: %s %%", humidity); 

  u8g2.drawStr(0, 10, tempDisplay);
  u8g2.drawStr(0, 25, humDisplay);

  u8g2.sendBuffer();
  //delay(3000);
}


































