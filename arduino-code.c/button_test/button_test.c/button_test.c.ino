#include <U8g2lib.h>
#include <Wire.h>
#include <WiFi.h>
#include <esp_wifi.h>
#define WIFI_SSID "CasaAraujo_East_5G"
#define WIFI_PASSWORD "quintinha2020top"

WiFiClient wifiClient;

int button = 4;
int buttonNew = 0;
int buttonOld=1;
int isShowingMAc=0;


U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

void setup() {
  Serial.begin(9600);
  pinMode(button, INPUT_PULLUP);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED)
  {
      Serial.print(".");
      delay(300);
  }

  //setups
  setupMonitor();

}



void loop() {
  buttonNew=digitalRead(button);

  if(buttonNew == LOW){
    Serial.println("Showing MAC");
    readMacAddress();
    delay(3000);
  }else{
    showSensorValues();
    delay(3000);
  }
  
  
  delay(100);
}


void showSensorValues() {
  float temp = 30; 
  float hum = 52;
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
  delay(3000);
}


void setupMonitor()
{
  u8g2.begin();
  u8g2.clearDisplay();
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.drawStr(3, 10, "Starting...");
  u8g2.sendBuffer();
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
