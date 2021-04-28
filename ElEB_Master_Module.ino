#include <SPI.h>
#include <Wire.h>
#include <Thread.h>
#include <WiFiNINA.h>
#include <ArduinoBLE.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <StaticThreadController.h>

#include "wiring_private.h"

#define MAX_CURRENT 500
#define MQTT_SERVER_IP "broker.emqx.io"
#define MQTT_CLIENT_ID "tleb_"
#define MQTT_SUB_TOPIC "smarthome/tleb/1"

/***
   Global Parameters
*/
String wifiType, wifiSSID, wifiUser, wifiPass;
bool advSMF = false;
int sysCurrent = 0;
int lastPlugAddr = 0;
int smfImportances[50] = {0};

TwoWire i2cWire(&sercom1, 11, 13);

/***
   Pin Settings
*/
int conncLedPin = 9;
int powerLedPin = 7;

/***
   ＭQTT Parameters
*/
WiFiClient mqttClient;
PubSubClient client(mqttClient);

/***
   Thread Instances
*/

Thread* i2cThread = new Thread();
Thread* mqttThread = new Thread();
Thread* smfThread = new Thread();
StaticThreadController<3> threadControl (i2cThread, mqttThread, smfThread);

void setup() {
  Serial.begin(9600);

  //wifiSSID = "Edwin's Room";
  //wifiPass = "Edw23190";
  //wifiSSID = "network";
  //wifiPass = "nkuste215@1";
  wifiSSID = "Cylu.iPhone.12";
  wifiPass = "Hello123";

  pinInit();
  wifiInit();
  i2cInit();
  mqttInit();

  i2cThread->onRun(i2cLoop);
  i2cThread->setInterval(180);

  mqttThread->onRun(mqttLoop);
  mqttThread->setInterval(3000);

  smfThread->onRun(smfLoop);
  smfThread->setInterval(100);
}

void loop() {
  checkWiFi();
  //bleConncLoop();
  threadControl.run();
  client.loop();
  delay(20);
}

void pinInit() {
  pinMode(conncLedPin, OUTPUT);
  pinMode(powerLedPin, OUTPUT);

  digitalWrite(conncLedPin, LOW);
  digitalWrite(powerLedPin, HIGH);
}
