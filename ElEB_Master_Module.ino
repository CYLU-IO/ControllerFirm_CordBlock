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
#define LONG_PRESS_TIME 10000

#define RST_PIN 2
#define BUTTON_PIN 3
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
    acc_info.initialized = true;
    //acc_info_flash.write(acc_info);
  }

  wifi_setting.ssid = "Edwin's Room";
  wifi_setting.password = "Edw23190";
  
  serialInit();
  while(!Serial);

  pinInit();
  resetToFactoryDetect();
  
  i2cInit();

  wifiInit();
  //mqttInit();
  clearSerial1();
  moduleReconncTrial();

  /*** HOMEKIT INIT ***/
  Serial.print(F("[Homekit] Initialize HAP: "));
  Serial.println(Homekit.init());

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
  digitalWrite(RST_PIN, HIGH);

  pinMode(MODULES_CONNC_STATE_PIN, OUTPUT);
  pinMode(WIFI_STATE_PIN, OUTPUT);
  pinMode(RST_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT);

  digitalWrite(MODULES_CONNC_STATE_PIN, LOW);
  digitalWrite(WIFI_STATE_PIN, LOW);
}

void resetToFactoryDetect() {
  if (digitalRead(BUTTON_PIN) == LOW) {
    unsigned long pressedTime = millis();

    digitalWrite(WIFI_STATE_PIN, HIGH);
    digitalWrite(MODULES_CONNC_STATE_PIN, HIGH);

    while (digitalRead(BUTTON_PIN) == LOW) {
      long pressDuration = millis() - pressedTime;

      if (pressDuration > LONG_PRESS_TIME) {
        //Start factory resetting
        Serial.println("Reset to factory...");
        delay(1000);
      }
    }
  }

  //blinkLED(3);
}

void resetFunc() {
  digitalWrite(RST_PIN, LOW);
}
