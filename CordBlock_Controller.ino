#include <CRC.h>
#include <CRC8.h>
#include <Button2.h>
#include <CoreBridge.h>
#include <ArduinoJson.h>

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
  bool module_connected;
} sys_status;

Uart Serial3 (&sercom0, 5, 6, SERCOM_RX_PAD_1, UART_TX_PAD_0);

void setup() {
  /*** Warehouse Init. for RingEEPROM and Sensor ***/
  Warehouse.begin();

  /*** UART and CoreBridge(SPI) Init. ***/
  serialInit();
  CoreBridge.begin();

  /*** Pin and Button Init. ***/
  pinInit();
  resetToFactoryDetect();

  //TEST ONLY- Remove
  //while (!Serial);
  SerialNina.begin(9600);
  SerialNina.begin(115200);
  //

  if (false) {
    Serial.println("[Warehouse] Clearing storage");
    Warehouse.clearStorage();

    /*Serial.println("[Warehouse] Appending fake data");
      for (int i = 0; i < 14; i++) {
      Warehouse.appendData(random(100, 15000));
      }

      Serial.print("[Warehouse] Current head: ");
      Serial.println(Warehouse.getHeadAddr());*/
  }

  /*** Action in setup() ***/
  moduleReconncTrial();
}

void loop() {
  if (Serial.available()) { //for test only
    int c = Serial.read();

    if (c == 87) { //W
      Serial.println("Fomatting");
      char *p = (char*)malloc(4 * sizeof(char));
      for (int i = 0; i < 4; i++) p[i] = i + 1;
      sendResetModule(Serial3, p, 4); //using boardcast
      free(p);

      //delay(3000);
      //Serial.println("Reconnect");
      //sendAddress(Serial1);
    }
  }

  //if (SerialNina.available()) Serial.write(SerialNina.read());

  configurationsUpdateLoop();

  wifiLedCheckRoutine();

  warehouseRequestCheckRoutine();

  receiveSerial();

  smfCheckRoutine();

  periodicCurrentRequest();

  moduleDataUpdateLoop();

  ModuleLiveCheckRoutine();

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
