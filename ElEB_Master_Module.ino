#include <Wire.h>
#include <Thread.h>
#include <WiFiNINA.h>
#include <ArduinoBLE.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <FlashStorage.h>
#include <ArduinoHomekit.h>
#include <StaticThreadController.h>

#include "wiring_private.h"

#define UNSET_ADDR 51
#define MAX_MODULES 50
#define MAX_CURRENT 500

#define WIFI_STATE_PIN 7
#define MODULES_CONNC_STATE_PIN 9

typedef struct {
  char* ssid;
  char* user;
  char* password;
  int type;
} WiFi_Setting;

typedef struct {
  char serial_number[12];
  char name[32];
  char code[10];
  char setupId[4];
  boolean initialized;
} Accessory_Info;

typedef struct {
  int all_current;
  int last_plugged;
  int num_modules;
  int modules[5][3]; //slave address[id][switchState][current]
} System_Info;

typedef struct {
  int importances[50];
  bool advancedSMF;
} Smart_Modularized_Fuse_Info;

/*** in-Flash Data ***/
FlashStorage(acc_info_flash, Accessory_Info);
FlashStorage(smf_info_flash, Smart_Modularized_Fuse_Info);

/*** Global Data ***/
WiFi_Setting wifi_setting;
Accessory_Info acc_info;
System_Info sys_info;
Smart_Modularized_Fuse_Info smf_info;

/*** ＭQTT ***/
WiFiClient mqttClient;
PubSubClient client(mqttClient);

/*** Thread Instances ***/
Thread* mqttThread = new Thread();
Thread* smfThread = new Thread();
StaticThreadController<2> threadControl (mqttThread, smfThread);

void setup() {
  acc_info = acc_info_flash.read();
  smf_info = smf_info_flash.read();

  if (!acc_info.initialized) { //Start Accessory Initialization
    serialNumGenerator(acc_info.serial_number, "TW0", "1", "3", 7);
    strcpy(acc_info.name, "TLEBV01TWA");
    strcpy(acc_info.code, "123-21-123");
    strcpy(acc_info.setupId, "CY1U");
    acc_info.initialized = true;
    //acc_info_flash.write(acc_info);
  }

  wifi_setting.ssid = "Edwin's Room";
  wifi_setting.password = "Edw23190";

  //wifiSSID = "network";
  //wifiPass = "nkuste215@1";
  //wifiSSID = "Cylu.iPhone.12";
  //wifiPass = "Hello123";

  serialInit();
  //while(!Serial);

  pinInit();
  i2cInit();

  wifiInit();
  //mqttInit();
  clearSerial1();
  moduleReconncTrial();

  //mqttThread->onRun(mqttLoop);
  //mqttThread->setInterval(3000);

  //smfThread->onRun(smfLoop);
  //smfThread->setInterval(100);
}

void loop() {
  //threadControl.run();

  checkWiFiConnc();
  serialSignalProcess();
  collectI2CData();
  checkSysCurrent();

  //client.loop();
  delay(1);
}

void pinInit() {
  pinMode(MODULES_CONNC_STATE_PIN, OUTPUT);
  pinMode(WIFI_STATE_PIN, OUTPUT);

  digitalWrite(MODULES_CONNC_STATE_PIN, LOW);
  digitalWrite(WIFI_STATE_PIN, LOW);
}
