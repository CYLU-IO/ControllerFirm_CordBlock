TwoWire i2cWire(&sercom1, 11, 13);

void i2cInit() {
  i2cWire.begin();

  pinPeripheral(11, PIO_SERCOM);
  pinPeripheral(13, PIO_SERCOM);
}

void collectI2CData() {
  String data;
  int newSysCurrent = 0;
  int current = 0;

  for (int i = 1; i <= numModule; i++) {
    if (sendCmd(i, "") != 0) continue; 
    
    data = receiveMsg(i, 9); //request data from slaves
    
    if (data.length() > 0) {
      //modules[i-1][0] = data.substring(0, 4).toInt(); //update id
      modules[i-1][1] = data.substring(4, 5).toInt(); //update switch state
      current = data.substring(5, 9).toInt(); //update current

      
      int hkState = Homekit.getState((uint8_t)i, (uint8_t)modules[i-1][0]);

      if (Homekit.getTriggerState((uint8_t)i, (uint8_t)modules[i-1][0])) { //if homekit event is triggere
        if (hkState) turnSwitchOn(i);
        else turnSwitchOff(i);
      } else {
        if (hkState != modules[i-1][1]) { //if homekit state isn't equal as module's state
          Homekit.setState((uint8_t)i, (uint8_t)modules[i-1][0], (uint8_t)modules[i-1][1]); //set homekit state forcibly
        }
      }

      if (max(current - modules[i-1][2] - 10, 0) > 0) lastPlugAddr = i; //current rises in a big ratio

      modules[i-1][2] = current;
      newSysCurrent += modules[i-1][2];
    } else {
      //Slave doesn not send any data, do checking
      //cleanDumpedModulesData(i);
    }

    delay(70);
  }

  sysCurrent = newSysCurrent; //update system current
}

void cleanDumpedModulesData(int addr) {
  modules[addr - 1][0] = 0;
  modules[addr - 1][1] = 0;
  modules[addr - 1][2] = 0;
}

String receiveMsg(int addr, int mlength) {
  String re = "";

  i2cWire.requestFrom(addr, mlength);

  while (i2cWire.available()) {
    char b = i2cWire.read();
    re += b;
  }

  return re;
}

int sendCmd(int addr, String rawCmd) {
  char * buf = (char *) malloc (rawCmd.length());
  rawCmd.toCharArray(buf, rawCmd.length() + 1);

  i2cWire.beginTransmission(addr);
  i2cWire.write(buf);
  free(buf);

  return i2cWire.endTransmission();
}
