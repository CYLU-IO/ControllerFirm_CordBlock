void homekitLoop() {
  if (!sys_status.module_initialized) return;

  static char cmd[MAX_MODULES * 2]; //(addr, action) * MAX_MODULES
  static int length = 0;
  static bool acted = false;

  for (uint8_t i = 0; i < sys_status.num_modules; i++) {
    int targetedAddr = i + 1;
    int mSwitchState = sys_status.modules[i][1];

    receiveSerial();

    if (CoreBridge.readModuleTriggered(i)) { //triggered, update module
      cmd[length * 2] = targetedAddr;

      if (CoreBridge.getModuleSwitchState(i)) {
#if DEBUG
        Serial.println("[HOMEKIT] Switch turn ON");
#endif
        cmd[length * 2 + 1] = DO_TURN_ON;
      }
      else {
#if DEBUG
        Serial.println("[HOMEKIT] Switch turn OFF");
#endif
        cmd[length * 2 + 1] = DO_TURN_OFF;
      }

      length++;
      acted = true;
    }
  }

  if (acted) {
    int l = length * 2;
    char *p = (char*)malloc(l * sizeof(char));

    for (int i = 0; i < l; i++) p[i] = cmd[i];

    sendCmd(Serial3, CMD_DO_MODULE, p, l);
    free(p);

    acted = false;
    length = 0;
  }
}

void moduleDataUpdateLoop() {
  if (!sys_status.module_initialized) return;

  for (uint8_t i = 0; i < sys_status.num_modules; i++) {
    int targetedAddr = i + 1;
    int mPriority = sys_status.modules[i][0];
    int spiPriority = CoreBridge.getModulePriority(i);

    receiveSerial();

    if (spiPriority != mPriority) {
      int a[1] = {targetedAddr};
      int v[1] = {spiPriority};

      sendUpdateData(Serial3, MODULE_PRIORITY, a, v, 1);
      sys_status.modules[i][0] = spiPriority;
    }
  }
}

void configurationsUpdateLoop() {
  device_config.advanced_smf = CoreBridge.getEnablePOP();
}

void wifiLedCheckRoutine() {
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
}

void warehouseRequestCheckRoutine() {
  int warehouse_request = CoreBridge.readWarehouseRequest();

  switch (warehouse_request) {
    case 0x01:
      CoreBridge.setWarehouseLength(Warehouse.getAvailableLength());
      break;

    default:
      if (warehouse_request > 0x01) {
        int buf[144];
        int buf_length = 144;

        Warehouse.getDataByPage(warehouse_request - 0x02, buf_length, buf);
        CoreBridge.setWarehouseBuffer((uint16_t *)buf, buf_length);
      }
  }
}

void turnSwitchOn(int addr) {
  char p[2] = {addr, DO_TURN_ON};
  sendCmd(Serial3, CMD_DO_MODULE, p, sizeof(p));
}
void turnSwitchOff(int addr) {
  char p[2] = {addr, DO_TURN_OFF};
  sendCmd(Serial3, CMD_DO_MODULE, p, sizeof(p));
}

void smfCheckRoutine() {
  if (sys_status.sum_current > MAX_CURRENT) {
    if (!smf_status.emerg_triggered) {
      if (device_config.advanced_smf) {
        int highest_priority = 0;

        for (int i = 0; i < sys_status.num_modules; i++) {
          if (sys_status.modules[i][1] && sys_status.modules[i][0] >= highest_priority) {
            smf_status.overload_triggered_addr = i + 1;
            highest_priority = sys_status.modules[i][0];
          }
        }
      }

#if DEBUG
      Serial.print("[SMF] Overloaded. Turning OFF: ");
      Serial.println(smf_status.overload_triggered_addr);
#endif

      turnSwitchOff(smf_status.overload_triggered_addr);
      smf_status.emerg_triggered = true;
    } else {
      static unsigned long t;

      if (millis() - t > 1000) {
        sendReqData(Serial3, MODULE_CURRENT);
        t = millis();
      }
    }
  } else {
    smf_status.emerg_triggered = false;
  }
}

void resetToFactoryDetect() {
  if (digitalRead(BUTTON_PIN) == LOW) {
    unsigned long pressedTime = millis();

    digitalWrite(WIFI_STATE_PIN, HIGH);
    digitalWrite(MODULES_STATE_PIN, HIGH);

    while (digitalRead(BUTTON_PIN) == LOW) {
      long pressDuration = millis() - pressedTime;

      if (pressDuration > LONG_PRESS_TIME) {
        digitalWrite(WIFI_STATE_PIN, LOW);

        CoreBridge.resetToFactory();
        resetSAMD21();
      }
    }
  }
}

void periodicCurrentRequest() {
  static unsigned long t;

  if (millis() - t >= PERIODID_CURRENT_TIME) { //5 minutes interval
    Serial.println("\nperiodicCurrentRequest Triggered");
    int buf[1] = {sys_status.sum_current};
    CoreBridge.setWarehouseBuffer((uint16_t *)buf, 1);

    Warehouse.appendData(sys_status.sum_current);
    sendReqData(Serial3, MODULE_CURRENT);

    t = millis();
  }
}

void resetSAMD21() {
  digitalWrite(RST_PIN, LOW);
}
