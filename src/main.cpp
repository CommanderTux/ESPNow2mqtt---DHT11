#include <Arduino.h>
#include "DHT.h"
#include <EspNow2MqttClient.hpp>

byte sharedKey[16] = {10,200,23,4,50,3,99,82,39,100,211,112,143,4,15,106};
byte sharedChannel = 11;
uint8_t gatewayMac[6] = {0x3C, 0x71, 0xBF, 0xC0, 0x3E, 0x4C};

EspNow2MqttClient client = EspNow2MqttClient("dht11", sharedKey, gatewayMac, sharedChannel);

bool weHaveResponse = false;
RTC_DATA_ATTR long timeEnd = 0;
RTC_DATA_ATTR int counter = 0;
#define SLEEP_SECS 15 * 60 // 15 minutes
#define SEND_TIMEOUT 245   // 245 millis seconds timeout 
#define ANALOGPIN 36       // Analog GPIO pin
#define DHTPIN 4           // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11      // DHT 11

//Create an instance of the object
DHT dht(DHTPIN, DHTTYPE);

float temp, humi, volt;
char tempChar[8], humiChar[8], voltChar[8];

void readDHT() {
  temp = dht.readTemperature();
  humi = dht.readHumidity();

  volt = analogRead(ANALOGPIN) / 216.0;
  sprintf(tempChar, "%.1f", temp);
  sprintf(humiChar, "%.1f", humi);
  sprintf(voltChar, "%.1f", volt);
  Serial.printf("voltage=%.3f, temperature=%.3f, humidity=%.3f\n", volt, temp, humi);
}

void gotoSleep(){
  timeEnd = millis();
  long timeMicros = SLEEP_SECS + ((uint8_t)esp_random()/2);
  esp_sleep_enable_timer_wakeup(timeMicros * 1000000);
  Serial.printf("Up for %lu ms, going to sleep for %lu secs...\n", millis(), timeMicros);
  esp_deep_sleep_start();
}
 
void setup() {
  Serial.begin(115200); Serial.println();

  dht.begin();

  // read sensor first before awake generates heat
  client.init();
  readDHT();
  client.doSend(tempChar,"temp");
  client.doSend(humiChar,"humi");
  client.doSend(voltChar,"volts");
}

void loop() {
    if(weHaveResponse || millis() > SEND_TIMEOUT){
      gotoSleep();
    }
    delay(10);
}
