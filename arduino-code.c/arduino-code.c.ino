#include <Arduino.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <PubSubClient.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"
#include <U8g2lib.h>
#include <esp_wifi.h>
#include <Wire.h>
#include <ESP32Servo.h>
#include "DHT.h"

//Wifi
#define WIFI_SSID "CasaAraujo_East_5G"
#define WIFI_PASSWORD "quintinha2020top"

//Firebase
#define API_KEY "AIzaSyDB8cIkBU5lxgIqYjr7YKDWvtlyf5SIeQ4"
#define DATABASE_URL "https://scmu-7d9e7-default-rtdb.europe-west1.firebasedatabase.app/" 
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

//MQTT
#define MQTT_BROKER_ADRESS "broker.hivemq.com"
#define MQTT_CLIENT_NAME "botaniq_client"
#define MQTT_PORT 1883
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

//Monitor
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

//Pins
#define BUTTONPIN 4
#define DHTPIN 19
#define LIGHTSENSORPIN 34
#define WATERLEVELPIN 33
#define SOILSENSORPIN 32
#define SERVO_PIN 18

//Servo
Servo myServo;

//Button
int buttonNew = 0;

//DHT
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
float lastTemp = 0.0;
float lastHum = 0.0;

void setup() 
{
  Serial.begin(9600);

  //Setups
  setupWifi();
  setupFirebase();
  setupMQTT();
  setupMonitor();
  setupButton();
  setupServo();
  const char* mac = getMacAddress();
  Serial.println(mac);
}

void loop() 
{
  handleMQTT();
  handleButton();//responsable to Display the values on the screen

  
}

void setupServo()
{
  myServo.attach(SERVO_PIN);
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

    /* Codigo para girar o servo
    Serial.println("Vai rodar");

    for (int angle = 0; angle <= 180; angle++) {
      myServo.write(angle);
      delay(15);  
    }

    delay(1000); 

    
    for (int angle = 180; angle >= 0; angle--) {
      myServo.write(angle);
      delay(15);
    }

    delay(1000);
    */
    
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

//Display all sensor values
void showSensorValues() {
  float temp = dht.readTemperature(); 
  float hum = dht.readHumidity();
  float light = getLightSensorValue();
  int waterLevel = getWaterLevel();
  int soilHum = getSoilHum();
  
  if (!isnan(temp) && !isnan(hum)) {
    lastTemp = temp;
    lastHum = hum;
  }

  
  u8g2.setFontPosCenter();
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x10_tr);
  //u8g2.setFont(u8g2_font_5x7_tr);

  char temperature[10];
  dtostrf(lastTemp, 6, 2, temperature);

  char humidity[10];
  dtostrf(lastHum, 6, 2, humidity);

  char tempDisplay[20];
  snprintf(tempDisplay, sizeof(tempDisplay), "Temp: %s C", temperature);

  char humDisplay[20];
  snprintf(humDisplay, sizeof(humDisplay), "Hum: %s %%", humidity); 

  char lightDisplay[20];
  snprintf(lightDisplay, sizeof(lightDisplay), "Light: %.1f %%", light);

  char waterDisplay[20];
  snprintf(waterDisplay, sizeof(waterDisplay), "W. Level: %d %%", waterLevel);

  char soilHumDisplay[20];
  snprintf(soilHumDisplay, sizeof(soilHumDisplay), "S. Hum: %d %%", soilHum);

  u8g2.drawStr(0, 11, tempDisplay);
  u8g2.drawStr(0, 22, humDisplay);
  u8g2.drawStr(0, 33, lightDisplay);
  u8g2.drawStr(0, 44, waterDisplay);
  u8g2.drawStr(0, 55, soilHumDisplay);

  u8g2.sendBuffer();
}

float getLightSensorValue() 
{
  int analogValue = analogRead(LIGHTSENSORPIN);
  //Serial.println(analogValue);
  float lightPercent = (analogValue / 4095.0) * 100.0;
  return lightPercent;
}

int getWaterLevel()
{
  int waterLevelRaw = analogRead(WATERLEVELPIN);
  int waterLevelPercent = map(waterLevelRaw, 0, 4095, 0, 100); 

  //Serial.println(waterLevelRaw);

  return waterLevelPercent;
}
 
int getSoilHum()
{
  int sensorValue = analogRead(SOILSENSORPIN);
  //Serial.println(sensorValue);
  int soilHumPercent = map(sensorValue, 0, 4095, 0, 100);

  return soilHumPercent;
}


































