void turnSwitchOn(int addr) {
  sendCmd(addr, "1");
}
void turnSwitchOff(int addr) {
  sendCmd(addr, "2");
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
