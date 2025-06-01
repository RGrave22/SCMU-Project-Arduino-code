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
#include <Preferences.h>

//Wifi
#define WIFI_SSID "CasaAraujo_East_5G"
#define WIFI_PASSWORD "quintinha2020top"
//#define WIFI_SSID "Grave"
//#define WIFI_PASSWORD "grave123"

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

//Preferences
Preferences preferences;
String myid;

//OLED
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

//Pins
#define BUTTONPIN 4
#define DHTPIN 19
#define LIGHTSENSORPIN 34
#define WATERLEVELPIN 33
#define SOILSENSORPIN 32
#define SERVO_PIN 18
#define WATERPUMPPIN 2

//Servo
Servo myServo;
int currentAngle = 0;
unsigned long lastServoTime = 0;
const unsigned long servoCooldown = 300000;
//const unsigned long servoCooldown = 15000;
bool firstServoUse = true;

//Button
int buttonNew = 0;

//DHT
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
float lastTemp = 0.0;
float lastHum = 0.0;

//Connection bools
bool hasWifi;

//waterPump
unsigned long lastWateringTime = 0;
const unsigned long wateringCooldown = 300000;
//const unsigned long wateringCooldown = 15000;
bool firstWaterUse = true;

void setup() 
{
  Serial.begin(9600);

  //Setups
  setupWifi();

  if(hasWifi){
    const char* mac = getMacAddress();
    setupFirebase();
    setupMQTT();
    Serial.println(mac);
  }



  setupMonitor();
  setupButton();
  setupServo();
  setupPreferences();
  setupWPump();
  
  
}

void loop() 
{
  if(hasWifi){
    handleMQTT();
  }
  
  handleButton(); //responsable to Display the values on the screen

}

void setupWPump()
{
  pinMode(WATERPUMPPIN, OUTPUT);
}

void setupServo()
{
  myServo.attach(SERVO_PIN);
}

/*
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
*/

void setupWifi() 
{
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");

  unsigned long startAttemptTime = millis();  // regista o tempo de início
  const unsigned long timeout = 10000;        // 10 segundos (em milissegundos)

  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < timeout) {
    Serial.print(".");
    delay(300);
  }

  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Connected with IP: " + WiFi.localIP().toString());
    hasWifi = true;
  } else {
    Serial.println("Failed to connect to Wi-Fi within 10 seconds.");
    hasWifi = false;
  }
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

  
    if (Firebase.ready()) {
      const char* mac = getMacAddress();
      String smac = String(mac);
      smac.trim();  // Remove espaços e quebras

      String path = "/macs/" + smac;

      if (Firebase.RTDB.getBool(&fbdo, path + "/hasUser")) {
          bool hasUser = fbdo.boolData();
          
          if (!hasUser) {
              FirebaseJson json;
              json.set("mac", smac);
              json.set("hasUser", false);

              if (Firebase.RTDB.setJSON(&fbdo, path, &json)) {
                  Serial.println("MAC address registered successfully");
              } else {
                  Serial.println("Failed to register MAC address: " + fbdo.errorReason());
              }
          } else {
              Serial.println("MAC already registered with hasUser true");
          }
      } else {
          
          FirebaseJson json;
          json.set("mac", smac);
          json.set("hasUser", false);

          if (Firebase.RTDB.setJSON(&fbdo, path, &json)) {
              Serial.println("MAC address registered successfully");
          } else {
              Serial.println("Failed to register MAC address: " + fbdo.errorReason());
          }
          
      }
  }
 
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

void setupPreferences()
{
    preferences.begin("mydata", false);
    //Uncomment if we want to reset the flash memory
    //preferences.clear();
    myid = preferences.getString("myid", "");
    Serial.print("Loaded myid: ");
    Serial.println(myid);
    
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

    
    //topic.replace(":", "");

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

    
    if (content.indexOf("turnwater") != -1) {
        Serial.println(content);
        String valueStr = content.substring(content.indexOf(":") + 1);
        int sec = valueStr.toInt();
        
        turnWaterPumpOn(sec);
    }

    if (content.indexOf("window") != -1) {
        Serial.println(content);
        turnServoOn();
    }

    if (content.indexOf("yourid") != -1) {
        myid = content.substring(content.indexOf(":") + 1);
        Serial.print("New myid received: ");
        Serial.println(myid);
        
        preferences.putString("myid", myid);
        Serial.println("myid saved to flash.");
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
    if(hasWifi){
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
    }else{
      u8g2.begin();
      u8g2.clearDisplay();
      u8g2.clearBuffer();
      u8g2.setFont(u8g2_font_6x10_tr); 
      u8g2.drawStr(0, 10, "No connection...");  
      u8g2.sendBuffer();
    }
    
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
    sprintf(s, "%02x:%02x:%02x:%02x:%02x:%02x",
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



  if(hasWifi) {
    sendtofbSensorValues(lastTemp, lastHum, light, waterLevel, soilHum);
    checkTresholds(lastTemp, lastHum, light, waterLevel, soilHum);
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


  //handling when there is no connection
  if(!hasWifi){
    if(soilHum < 30){
      if(waterLevel != 0){
          turnWaterPumpOnAsRoutine(4);
      }
      
    }
    if(waterLevel < 10){
      u8g2.setFontPosCenter();
      u8g2.clearBuffer();

      u8g2.drawStr(0, 11, "Low water level...");
      u8g2.sendBuffer();
      delay(2000);
    }
    if(lastHum > 80){
      turnServoOnAsRoutine();
    }
    if(lastHum < 30){
      turnServoOnAsRoutine();
    }
  }
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

void turnWaterPumpOn(int sec)
{
  
  u8g2.setFontPosCenter();
  u8g2.clearBuffer();
  u8g2.drawStr(0, 11, "Watering...");
  u8g2.sendBuffer();
  digitalWrite(WATERPUMPPIN, HIGH);
  int milisec = sec*1000;
  delay(milisec);
  digitalWrite(WATERPUMPPIN, LOW);
  //Serial.println(myid);
}

void turnWaterPumpOnAsRoutine(int sec)
{
  unsigned long now = millis();

  if (firstWaterUse || (now - lastWateringTime >= wateringCooldown)) {
    u8g2.setFontPosCenter();
    u8g2.clearBuffer();
    u8g2.drawStr(0, 11, "Watering...");
    u8g2.sendBuffer();

    digitalWrite(WATERPUMPPIN, HIGH);
    delay(sec * 1000);
    digitalWrite(WATERPUMPPIN, LOW);

    lastWateringTime = now;  
    firstWaterUse = false;
    Serial.println("Watered at: " + String(now / 1000) + "s");
  } else {
    Serial.println("Watering skipped: cooldown not finished.");
  }
}



/*
void turnServoOn(int angleToGet)
{
  u8g2.setFontPosCenter();
  u8g2.clearBuffer();
  u8g2.drawStr(0, 11, "Opening window...");
  u8g2.sendBuffer();
  /*
  int maxAngle = angleToGet;

  int angle = 0;
  for (angle = 0; angle <= maxAngle; angle++) {
    myServo.write(angle);
    delay(15);  
  }

  delay(1000); 
  

  currentAngle += 45;
  if (currentAngle > 180) currentAngle = 180;
  myServo.write(currentAngle);

  delay(500);
}
*/

void turnServoOn()
{
  myServo.write(120);     
  delay(300);             
  myServo.write(95);
}

void turnServoOnAsRoutine()
{
  unsigned long now = millis();

  if (firstServoUse || (now - lastServoTime >= servoCooldown)) {
    myServo.write(120);     
    delay(300);             
    myServo.write(95);

    lastServoTime = now;
    firstServoUse = false;
    Serial.println("Servo activated.");
  } else {
    Serial.println("Servo skipped: cooldown not finished.");
  }
}

void sendtofbSensorValues(float temp, float hum, int light, int waterLevel, int soilHum) 
{
    String basePath = "/greenhouses/" + myid + "/";

    //Serial.println(myid);

    if(myid != ""){
      FirebaseJson json;
      json.set("temperature", temp);
      json.set("humidity", hum);
      json.set("light", light);
      json.set("waterLevel", waterLevel);
      json.set("soilHumidity", soilHum);

      //if (Firebase.RTDB.updateNode(&fbdo, basePath.c_str(), &json)) {
      if (Firebase.RTDB.updateNode(&fbdo, basePath, &json)) {
        Serial.println("Sensor values updated successfully.");
      } else {
        Serial.print("Firebase update failed: ");
        Serial.println(fbdo.errorReason());
      }
    }
}


void checkTresholds(float temp, float hum, int light, int waterLevel, int soilHum) {
  
  if(myid != ""){

      String pathHumidity = "/greenhouses/" + myid + "/thresholdHumidityMax";
      int maxHumidity = 0;
      if (Firebase.RTDB.getInt(&fbdo, pathHumidity)) {
        maxHumidity = fbdo.intData();
        Serial.printf("Max Humidity: %d\n", maxHumidity);

        if (hum > maxHumidity) {
          Serial.println("Opening window...");
          turnServoOnAsRoutine();
        }
      } else {
        Serial.println("Erro ao obter thHumidity: " + fbdo.errorReason());
      }

      String pathminHumidity = "/greenhouses/" + myid + "/thresholdHumidityMin";
      int minHumidity = 0;
      if (Firebase.RTDB.getInt(&fbdo, pathminHumidity)) {
        minHumidity = fbdo.intData();
        Serial.printf("Min Humidity: %d\n", minHumidity);

        if (hum < minHumidity) {
          Serial.println("Opening window...");
          turnServoOnAsRoutine();
        }

      } else {
        Serial.println("Erro ao obter thHumidity: " + fbdo.errorReason());
      }

      String pathSoilHumidity = "/greenhouses/" + myid + "/thresholdSoilHumidity";
      int maxSoilHumidity = 0;
      if (Firebase.RTDB.getInt(&fbdo, pathSoilHumidity)) {
        maxSoilHumidity = fbdo.intData();
        Serial.printf("Max Soil Humidity: %d\n", maxSoilHumidity);

        if (soilHum < maxSoilHumidity) {
          if(waterLevel!=0){
            Serial.println("Watering...");
            turnWaterPumpOnAsRoutine(4);
          }
          
        }
      } else {
        Serial.println("Erro ao obter thSoilHumidity: " + fbdo.errorReason());
      }
  }
  
}














