#include <CRC.h>
#include <CRC8.h>
#include <Wire.h>
#include <WiFiNINA.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <FlashStorage.h>
#include <ArduinoHomekit.h>

#include "wiring_private.h"

#define UNSET_ADDR 51
#define MAX_MODULES 20
#define MAX_CURRENT 500
#define LONG_PRESS_TIME 10000

#define RST_PIN 2
#define BUTTON_PIN 3
#define WIFI_STATE_PIN 7
#define MODULES_CONNC_STATE_PIN 9

/*** SERIAL ***/
#define CMD_FAIL            0x11
#define CMD_EOF             0x20
#define CMD_REQ_ADR         0x41 //'A'
#define CMD_LOAD_MODULE     0x42 //'B'
#define CMD_CONFIRM_RECEIVE 0x43 //'C'
#define CMD_DO_MODULE       0x44 //'D'
#define CMD_HI              0x48 //'H'
#define CMD_INIT_MODULE     0x49 //'I'
#define CMD_LINK_MODULE     0x4C //'L'
#define CMD_UPDATE_MASTER   0x55 //'U'
#define CMD_START           0xFF

/*** Module Actions ***/
#define DO_TURN_ON          0x6E //'n'
#define DO_TURN_OFF         0x66 //'f'

/*** Characteristic Type ***/
#define MODULE_SWITCH_STATE 0x61 //'a' 
#define MODULE_CURRENT      0x62 //'b'

struct WiFi_Setting {
  char* ssid;
  char* user;
  char* password;
  int type;
} wifi_setting;

struct Accessory_Info {
  char serial_number[12];
  char name[32];
  boolean initialized;
} acc_info;

struct System_Info {
  int all_current;
  int last_plugged;
  int num_modules;
  int modules[20][3]; //slave address[id][switchState][current]
} sys_info;

struct Smart_Modularized_Fuse_Info {
  int importances[20];
  bool advancedSMF;
} smf_info;

struct System_Status {
  bool module_initialized;
} sys_status;

/*** in-Flash Data ***/
FlashStorage(acc_info_flash, Accessory_Info);
FlashStorage(smf_info_flash, Smart_Modularized_Fuse_Info);

/*** ＭQTT ***/
WiFiClient mqttClient;
PubSubClient client(mqttClient);

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

  pinInit();
  resetToFactoryDetect();

  i2cInit();
  serialInit();
  //while (!Serial);
  moduleReconncTrial();

  wifiInit();
  //mqttInit();

  /*** HOMEKIT INIT ***/
  Serial.print(F("[HOMEKIT] Initialize HAP: "));
  Serial.println(Homekit.init());
}

void loop() {
  checkWiFiConnc();
  receiveSerial();
  homekitLoop();
  //checkSysCurrent();

  //client.loop();
  delay(1);
}

void pinInit() {
  digitalWrite(RST_PIN, HIGH);
  digitalWrite(BUTTON_PIN, HIGH);

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
