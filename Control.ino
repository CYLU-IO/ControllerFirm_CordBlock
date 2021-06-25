void homekitLoop() {
  if (!sys_status.module_initialized) return;

  static char cmd[MAX_MODULES * 2]; //(addr, action) * MAX_MODULES
  static int length = 0;
  static bool acted = false;

  for (uint8_t i = 0; i < sys_info.num_modules; i++) {
    int targetedAddr = i + 1;
    int mid = sys_info.modules[i][0];
    int mSwitchState = sys_info.modules[i][1];
    int hkState = Homekit.getServiceValue(i, mid);

    receiveSerial();

    if (Homekit.readServiceTriggered(i, mid)) { //triggered, update module
      hkState = Homekit.getServiceValue(i, mid);
      cmd[length * 2] = targetedAddr;

      if (hkState) {
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

#if ENABLE_I2C
    sendI2CCmd(CMD_DO_MODULE, p, l);
#else
    sendCmd(Serial3, CMD_DO_MODULE, p, l);
#endif

    free(p);

    acted = false;
    length = 0;
  }
}

void turnSwitchOn(int addr) {
  char p[1] = {addr};
  sendDoModule(Serial3, DO_TURN_ON, p, sizeof(p));
}
void turnSwitchOff(int addr) {
  char p[1] = {addr};
  sendDoModule(Serial3, DO_TURN_OFF, p, sizeof(p));
}

void checkSysCurrent() {
  if (sys_info.all_current > MAX_CURRENT) { //check if system current is over loaded
    if (smf_info.advancedSMF) { //if customized emergency cutdown is enabled
      /*
         1. reverse the smfImportances to start cutting down powered plug(check the current)
      */

      for (int i = sys_info.num_modules - 1; i >= 0; i--) {
        int addr = searchAddrById(smf_info.importances[i]);

        if (sys_info.modules[addr][1] && sys_info.modules[addr][2] > 10) {
          turnSwitchOff(addr);
        }
      }
    } else {
      Serial.println("System current is overloaded! Cut down the last-plugged.");
      if (sys_info.last_plugged != 0) turnSwitchOff(sys_info.last_plugged); //cut the overloaded itself
    }
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

  if (millis() - t > PERIODID_CURRENT_TIME) { //5 minutes interval
    sendReqData(Serial3, MODULE_CURRENT);

    t = millis();
  }
}

void resetSAMD21() {
  digitalWrite(RST_PIN, LOW);
}
