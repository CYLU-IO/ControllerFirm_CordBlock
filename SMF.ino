void smfLoop() {
  checkSysCurrent();
}

void checkSysCurrent() {
  if (sysCurrent > MAX_CURRENT) { //check if system current is over loaded
    if (advSMF) { //if customized emergency cutdown is enabled
      /* 
       * 1. reverse the smfImportances to start cutting down powered plug(check the current)
       */

       for (int i = 49; i >= 0; i--) {
          int addr = searchAddrById(smfImportances[i]);

          if (slaves[addr][1] && slaves[addr][2] > 10) {
            sendCmd(addr, "5");
          }
       }
    } else {
      Serial.println("System current is overloaded! Cut down the last-plugged.");
      if (lastPlugAddr != 0) sendCmd(lastPlugAddr, "5"); //cut the overloaded itself
    }
  }
}
