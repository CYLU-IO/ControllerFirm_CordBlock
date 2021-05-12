#include <Wire.h>
#include <Thread.h>
#include <WiFiNINA.h>
#include <ArduinoBLE.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <ArduinoHomekit.h>

#include <StaticThreadController.h>

#include "wiring_private.h"       

#define UNSET_ADDR 51
#define MAX_MODULES 50
#define MAX_CURRENT 500
#define MQTT_SERVER_IP "broker.emqx.io"
#define MQTT_CLIENT_ID "tleb_"
#define MQTT_SUB_TOPIC "smarthome/tleb/1"

/***
   Global Parameters
*/
String wifiType, wifiSSID, wifiUser, wifiPass;
char serial_num[12];
const char* acc_name;
const char* acc_code;
const char* acc_setupId;

bool advSMF = false;
int sysCurrent = 0;
int lastPlugAddr = 0;
int numModule = 0;
int modules[50][3] = {0}; //slave address[id][switchState][current]
int smfImportances[50] = {0};

/***
   Pin Settings
*/
int conncLedPin = 9;
int wifiLedPin = 7;

/***
   ＭQTT Parameters
*/
WiFiClient mqttClient;
PubSubClient client(mqttClient);

/***
   Thread Instances
*/

Thread* mqttThread = new Thread();
Thread* smfThread = new Thread();
StaticThreadController<2> threadControl (mqttThread, smfThread);

void setup() {
  
  wifiSSID = "Edwin's Room";
  wifiPass = "Edw23190";
  //wifiSSID = "network";
  //wifiPass = "nkuste215@1";
  //wifiSSID = "Cylu.iPhone.12";
  //wifiPass = "Hello123";

  serialInit();
  pinInit();
  i2cInit();
  wifiInit();
  clearSerial1();
  
  //mqttInit();

  //mqttThread->onRun(mqttLoop);
  //mqttThread->setInterval(3000);

  //smfThread->onRun(smfLoop);
  //smfThread->setInterval(100);

  /*** Homekit Info ***/
  serialNumGenerator(serial_num, "TW0", "1", "3", 7);
  acc_name = "串-智能電積木 Beta";
  acc_code = "123-21-123";
  acc_setupId = "CY1U";
}

void loop() {
  //checkWiFi();
  //bleConncLoop();
  //threadControl.run();
  serialSignalProcess();
  collectI2CData();
  //checkSysCurrent();
  //client.loop();
  delay(1);
}

void pinInit() {
  pinMode(conncLedPin, OUTPUT);
  pinMode(wifiLedPin, OUTPUT);

  digitalWrite(conncLedPin, LOW);
  digitalWrite(wifiLedPin, LOW);
}
