#include <U8g2lib.h>
#include <Wire.h>
#include "DHT.h"

//DHT
#define DHTPIN 4      // Pin connected to the sensor
#define DHTTYPE DHT11  // Use DHT11 or DHT22

DHT dht(DHTPIN, DHTTYPE);
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);


void setup() {
  Serial.begin(9600);
  dht.begin();

  u8g2.begin();
  u8g2.clearDisplay();
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.drawStr(3, 10, "Sempre a puxar!!!");
  u8g2.sendBuffer();

}

void loop()  {
   //DHT
  float temp = dht.readTemperature(); // Default is Celsius
  float hum = dht.readHumidity();

  Serial.print("Temp: ");
  Serial.print(temp);
  Serial.print("°C  Humidity: ");
  Serial.print(hum);
  Serial.println("%");

  u8g2.setFontPosCenter();

  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_10x20_te);
  char temperature[10];
  dtostrf(temp, 6, 2, temperature);
  char humidity[10];
  dtostrf(hum, 6, 2, humidity);

  u8g2.drawStr(0, 20, "TEMPERATURE");
  u8g2.drawStr(0, 40, strcat(temperature, " C"));
  u8g2.sendBuffer();

  delay(3500);

  u8g2.clearBuffer();
  u8g2.drawStr(0, 20, "HUMIDITY");
  u8g2.drawStr(0, 40, strcat(humidity, "%"));
  u8g2.sendBuffer();

  delay(3500);
}