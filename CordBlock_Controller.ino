#include <CRC.h>
#include <CRC8.h>
#include <Wire.h>
#include <Button2.h>
#include <ArduinoJson.h>
#include <FlashStorage.h>
#include <ArduinoHomekit.h>

#include "firm_definitions.h"
#include "wiring_private.h"

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

void setup() {
  acc_info = acc_info_flash.read();
  smf_info = smf_info_flash.read();

  if (!acc_info.initialized) { //Start Accessory Initialization
    serialNumGenerator(acc_info.serial_number, "TW0", "1", "3", 7);
    strcpy(acc_info.name, "CordBlock");
    acc_info.initialized = true;
    //acc_info_flash.write(acc_info);
  }

#if ENABLE_I2C_CMD
  i2cInit();
#endif

  serialInit();

#if ENABLE_HOMEKIT
  Homekit.init();
#endif

  pinInit();
  resetToFactoryDetect();

  //while (!Serial);

  //SerialNina.begin(115200);

  moduleReconncTrial();
}

void loop() {
  //checkSysCurrent();

  if (Serial.available()) {
    if (Serial.read() == 82) {
      Serial.println("[COM] Reset to factory");
      Homekit.resetToFactory();
    }
  }

  //if (SerialNina.available()) {
  //Serial.write(SerialNina.read());
  //}

  receiveSerial();

#if ENABLE_HOMEKIT
  homekitLoop();
#endif
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
        digitalWrite(WIFI_STATE_PIN, LOW);
        Homekit.resetToFactory();
        resetFunc();
      }
    }
  }
}

void resetFunc() {
  digitalWrite(RST_PIN, LOW);
}
