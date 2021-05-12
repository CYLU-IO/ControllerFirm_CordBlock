void turnSwitchOn(int addr) {
  sendCmd(addr, "1");
}
void turnSwitchOff(int addr) {
  sendCmd(addr, "2");
}

void boardcastWork() {
  int i;
  
  for (i = 1; i <= numModule; i++) sendCmd(i, "9");
}

void checkSysCurrent() {
  if (sysCurrent > MAX_CURRENT) { //check if system current is over loaded
    if (advSMF) { //if customized emergency cutdown is enabled
      /*
         1. reverse the smfImportances to start cutting down powered plug(check the current)
      */

      for (int i = 49; i >= 0; i--) {
        int addr = searchAddrById(smfImportances[i]);

        if (modules[addr][1] && modules[addr][2] > 10) {
          turnSwitchOff(addr);
        }
      }
    } else {
      Serial.println("System current is overloaded! Cut down the last-plugged.");
      if (lastPlugAddr != 0) turnSwitchOff(lastPlugAddr); //cut the overloaded itself
    }
  }
}
