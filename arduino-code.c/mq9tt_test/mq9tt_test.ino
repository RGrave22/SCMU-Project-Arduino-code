#include <WiFi.h>
#include <PubSubClient.h>

// WiFi credentials
#define WIFI_SSID "Grave"
#define WIFI_PASSWORD "grave123"

WiFiClient wifiClient;  // Regular WiFi client for MQTT
PubSubClient mqttClient(wifiClient);  // PubSubClient for MQTT

// MQTT constants
// broker address, port, and client name
const char *MQTT_BROKER_ADRESS = "broker.hivemq.com";
const uint16_t MQTT_PORT = 1883;
const char *MQTT_CLIENT_NAME = "botaniq_client";

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
    Serial.begin(9600);

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(300);
    }
    
    delay(1500);
    InitMqtt();
}

// only calls HandleMqtt
void loop()
{
    HandleMqtt();
}