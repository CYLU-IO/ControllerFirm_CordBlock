void homekitLoop() {
  if (!sys_status.module_initialized) return;

  for (uint8_t i = 0; i < sys_info.num_modules; i++) {
    int targetedAddr = i + 1;
    int mid = sys_info.modules[i][0];
    int mSwitchState = sys_info.modules[i][1];
    int hkState = Homekit.getServiceValue(i, mid);

    if (Homekit.getServiceTriggered(i, mid)) { //triggered, update module
      hkState = Homekit.getServiceValue(i, mid);
      
      if (hkState) {
        Serial.println("[HOMEKIT] Switch turn ON");
        turnSwitchOn(targetedAddr);
      }
      else {
        Serial.println("[HOMEKIT] Switch turn OFF");
        turnSwitchOff(targetedAddr);
      }

      sys_info.modules[i][1] = hkState;
    } else {
      if (hkState != mSwitchState) Homekit.setServiceValue(i, mid, mSwitchState); //set homekit state forcibly
    }
  }
}

void turnSwitchOn(int addr) {
  char p[1] = {addr};
  sendDoModule(Serial1, DO_TURN_ON, p, sizeof(p));
}
void turnSwitchOff(int addr) {
  char p[1] = {addr};
  sendDoModule(Serial1, DO_TURN_OFF, p, sizeof(p));
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
