#include <SPI.h>
#include <Wire.h>
#include <WiFiNINA.h>
#include <ArduinoBLE.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "wiring_private.h"

#define MAX_CURRENT 500

/***
   Global Parameters
*/
String wifiType, wifiSSID, wifiUser, wifiPass;

bool advSmtOverloader = false;
int sysCurrent = 0;
int lastPlugAddr = 0;

TwoWire i2cWire(&sercom1, 11, 13);

/***
   Pin Settings
*/
int conncLedPin = 9;
int powerLedPin = 7;

/***
   ＭQTT Parameters
*/
#define MQTT_SERVER_IP "broker.emqx.io"
#define MQTT_CLIENT_ID "tleb_"
#define MQTT_SUB_TOPIC "smarthome/tleb/1"

WiFiClient mqttClient;
PubSubClient client(mqttClient);

void setup() {
  Serial.begin(9600);

  wifiSSID = "Edwin's Room";
  wifiPass = "Edw23190";
  //wifiSSID = "network";
  //wifiPass = "nkuste215@1";

  sensInit();
  wifiConncInit();
  i2cManInit();
  //mqttInit();
}

void loop() {
  sensLoop();
  checkWiFiLed();
  i2cManLoop();
  //bleConncLoop();
  //mqttLoop();
  //client.loop();
  
  delay(100);
}
