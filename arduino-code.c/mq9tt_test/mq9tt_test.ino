#include <WiFi.h>
#include <PubSubClient.h>
#include <esp_wifi.h>
#include <U8g2lib.h>

// WiFi credentials
#define WIFI_SSID "Grave"
#define WIFI_PASSWORD "grave123"

WiFiClient wifiClient;               // Regular WiFi client for MQTT
PubSubClient mqttClient(wifiClient); // PubSubClient for MQTT

// MQTT constants
// broker address, port, and client name
const char *MQTT_BROKER_ADRESS = "broker.hivemq.com";
const uint16_t MQTT_PORT = 1883;
const char *MQTT_CLIENT_NAME = "botaniq_client";

U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

// subscribes to the topic
// in this example, only to 'hello/world'
void SuscribeMqtt()
{
    mqttClient.subscribe("cocomole");
}

// callback to execute when a message is received
// in this example, shows the received message by serial
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

// starts the MQTT communication
// starts setting the server and the callback when receiving a message
void InitMqtt()
{
    mqttClient.setServer(MQTT_BROKER_ADRESS, MQTT_PORT);
    mqttClient.setCallback(OnMqttReceived);
}

// connects or reconnects to MQTT
// connects -> subscribes to topic and publishes a message
// no -> waits 5 seconds
void ConnectMqtt()
{
    Serial.print("Starting MQTT connection...");
    if (mqttClient.connect(MQTT_CLIENT_NAME))
    {
        SuscribeMqtt();
        mqttClient.publish("connected", "hello/world");
    }
    else
    {
        Serial.print("Failed MQTT connection, rc=");
        Serial.print(mqttClient.state());
        Serial.println(" try again in 5 seconds");

        delay(5000);
    }
}

// manages MQTT communication
// checks if the client is connected
// no -> tries to reconnect
// yes -> calls the MQTT loop
void HandleMqtt()
{
    if (!mqttClient.connected())
    {
        ConnectMqtt();
    }
    mqttClient.loop();
}

// only starts serial, ethernet, and MQTT
void setup()
{
    Serial.begin(115200);

    readMacAddress();

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(300);
    }

    delay(1500);
    InitMqtt();
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

// only calls HandleMqtt
void loop()
{
    // HandleMqtt();
    // readMacAddress();
}