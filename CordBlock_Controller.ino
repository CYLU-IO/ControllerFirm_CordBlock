#include <CRC.h>
#include <CRC8.h>
#include <Wire.h>
#include <Button2.h>
#include <CoreBridge.h>
#include <ArduinoJson.h>
#include <FlashStorage.h>

#include "firm_definitions.h"
#include "wiring_private.h"

struct Accessory_Info {
  bool initialized;
} acc_info;

struct System_Info {
  int num_modules;
  int sum_current;
  int modules[20][3]; //modules DB[priority][switchState][current]
} sys_info;

struct Smart_Modularized_Fuse_Info {
  int  mcub;
  int  overload_triggered_addr;
  bool advanced_smf;
  bool emerg_triggered;
} smf_info;

struct System_Status {
  bool module_initialized;
} sys_status;

/*** in-Flash Data ***/
FlashStorage(acc_info_flash, Accessory_Info);
FlashStorage(smf_info_flash, Smart_Modularized_Fuse_Info);

Uart Serial3 (&sercom0, 5, 6, SERCOM_RX_PAD_1, UART_TX_PAD_0);

void setup() {
  acc_info = acc_info_flash.read();
  smf_info = smf_info_flash.read();

  if (!acc_info.initialized) {
    //serialNumGenerator(acc_info.serial_number, "TW0", "1", "3", 7);
    acc_info.initialized = true;
    //acc_info_flash.write(acc_info);
  }

  //TEST ONLY- Remove
  smf_info.advanced_smf = false;

#if ENABLE_I2C
  i2cInit();
#endif

  serialInit();
  CoreBridge.begin();

  pinInit();
  resetToFactoryDetect();

  while (!Serial);
  SerialNina.begin(115200);

  moduleReconncTrial();
}

void loop() {
  switch (WifiMgr.getStatus()) {
    case 3:
      digitalWrite(WIFI_STATE_PIN, HIGH);
      break;

    case 4:
      CoreBridge.resetNetwork();
      resetSAMD21();
      break;

    default:
      digitalWrite(WIFI_STATE_PIN, LOW);
      break;
  }

  if (Serial.available()) { //for test only
    int c = Serial.read();

    if (c == 87) { //W
      sendReqData(Serial3, MODULE_CURRENT);
      
      Serial.print("[UART] System current: ");
      Serial.println(sys_info.sum_current);

      Serial.print("[SMF] MCUB: ");
      Serial.println(smf_info.mcub);
    }
  }

  if (SerialNina.available()) Serial.write(SerialNina.read());

  receiveSerial();

  smartCurrentCheck();
  periodicCurrentRequest();

#if ENABLE_HOMEKIT
  homekitLoop();
#endif
}

void pinInit() {
  digitalWrite(RST_PIN, HIGH);
  digitalWrite(BUTTON_PIN, HIGH);

  pinMode(MODULES_STATE_PIN, OUTPUT);
  pinMode(WIFI_STATE_PIN, OUTPUT);
  pinMode(RST_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT);

  digitalWrite(MODULES_STATE_PIN, LOW);
  digitalWrite(WIFI_STATE_PIN, LOW);
}

void SERCOM0_Handler() {
  Serial3.IrqHandler();
}
