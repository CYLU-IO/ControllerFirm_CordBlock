#include <CRC.h>
#include <CRC8.h>
#include <Wire.h>
#include <Button2.h>
#include <CoreBridge.h>
#include <ArduinoJson.h>
#include <FlashStorage.h>

#include "firm_definitions.h"
#include "wiring_private.h"

struct device_config_t {
  bool advanced_smf;
  bool initialized;
} device_config;

struct smart_modularized_fuse_status_t {
  int  mcub;
  int  overload_triggered_addr;
  bool emerg_triggered;
} smf_status;

struct system_status_t {
  int num_modules;
  int sum_current;
  int modules[20][3]; //modules DB[priority][switchState][current]
  bool module_initialized;
} sys_status;

/*** in-Flash Data ***/
FlashStorage(device_config_flash, device_config_t);

Uart Serial3 (&sercom0, 5, 6, SERCOM_RX_PAD_1, UART_TX_PAD_0);

void setup() {
  /*** Device Configuration ***/
  device_config = device_config_flash.read();
  if (!device_config.initialized) {
    device_config.advanced_smf = false;
    
    device_config.initialized = true;
    //device_config_flash.write(device_config);
  }

  /*** I2C Init. for RingEEPROM and Sensor ***/
  i2cInit();

  /*** UART and CoreBridge(SPI) Init. ***/
  serialInit();
  CoreBridge.begin();

  /*** Pin and Button Init. ***/
  pinInit();
  resetToFactoryDetect();

  //TEST ONLY- Remove
  while (!Serial);
  SerialNina.begin(115200);
  //

  /*** Action in setup() ***/
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
      int a[1] = {1};
      int v[1] = {2};
      sendUpdateData(Serial3, MODULE_PRIORITY, a, v, 1);
    }
  }

  if (SerialNina.available()) Serial.write(SerialNina.read());

  receiveSerial();

  smartCurrentCheck();
  periodicCurrentRequest();

  moduleDataUpdateLoop();

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
