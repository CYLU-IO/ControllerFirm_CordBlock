void homekitLoop() {
  if (!sys_status.module_initialized) return;

  static char cmd[MAX_MODULES * 2]; //(addr, action) * MAX_MODULES
  static int length = 0;
  static bool acted = false;

  for (uint8_t i = 0; i < sys_info.num_modules; i++) {
    int targetedAddr = i + 1;
    int mSwitchState = sys_info.modules[i][1];

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

  for (uint8_t i = 0; i < sys_info.num_modules; i++) {
    int targetedAddr = i + 1;
    int mPriority = sys_info.modules[i][0];
    int spiPriority = CoreBridge.getModulePriority(i);

    receiveSerial();

    if (spiPriority != mPriority) {
      int a[1] = {targetedAddr};
      int v[1] = {spiPriority};
      
      sendUpdateData(Serial3, MODULE_PRIORITY, a, v, 1);
      sys_info.modules[i][0] = spiPriority;
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

void smartCurrentCheck() {
  if (sys_info.sum_current > MAX_CURRENT) {
    if (!smf_info.emerg_triggered) {
      if (smf_info.advanced_smf) {
        int highest_priority = 0;

        for (int i = 0; i < sys_info.num_modules; i++) {
          if (sys_info.modules[i][1] && sys_info.modules[i][0] >= highest_priority) {
            smf_info.overload_triggered_addr = i + 1;
            highest_priority = sys_info.modules[i][0];
          }
        }
      }

#if DEBUG
      Serial.print("[SMF] Overloaded. Turning OFF: ");
      Serial.println(smf_info.overload_triggered_addr);
#endif

      turnSwitchOff(smf_info.overload_triggered_addr);
      smf_info.emerg_triggered = true;
    } else {
      static unsigned long t;

      if (millis() - t > 1000) {
        sendReqData(Serial3, MODULE_CURRENT);
        t = millis();
      }
    }
  } else {
    smf_info.emerg_triggered = false;
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
    sendReqData(Serial3, MODULE_CURRENT);

    t = millis();
  }
}

void resetSAMD21() {
  digitalWrite(RST_PIN, LOW);
}
