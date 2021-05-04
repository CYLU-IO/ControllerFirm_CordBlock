TwoWire i2cWire(&sercom1, 11, 13);

void i2cInit() {
  i2cWire.begin();

  pinPeripheral(11, PIO_SERCOM);
  pinPeripheral(13, PIO_SERCOM);
}

void collectI2CData() {
  String data;
  int newSysCurrent, current = 0;

  for (int i = 1; i <= connectedSlave; i++) {
    data = receiveMsg(i, 9); //request data from slaves

    if (data.length() > 0) {
      modules[i][0] = data.substring(0, 4).toInt(); //update id
      modules[i][1] = data.substring(4, 5).toInt(); //update switch state
      current = data.substring(5, 9).toInt(); //update current

      if (current - modules[i][2] - 10 > 0) lastPlugAddr = i; //current rises in a big ratio

      modules[i][2] = current;
      newSysCurrent += modules[i][2];
    } else {
      //Slave doesn not send any data, do checking
    }
  }

  cleanDumpedModulesData();
  sysCurrent = newSysCurrent; //update system current
}

void cleanDumpedModulesData() {
  for (int i = connectedSlave - 1; i < MAX_MODULES; i++) {
    modules[i][0] = 0;
    modules[i][1] = 0;
    modules[i][2] = 0;
  }
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

/*** OLD I2C FUNCTIONS
  void initSlaves() { //ABANDON
  bool allUni = true;
  int newSysCurrent;

  do {
    int nearZero = findNearZero();
    newSysCurrent = 0;
    allUni = true;

    sendCmd(UNSET_ADDR, "1" + int2str(findNearZero() + 1, 2) + "50"); //announce to addressing randomly

    for (int i = 1; i < UNSET_ADDR; i++) {
      String returnData = receiveMessage(i, 12); //request data from slaves

      if (returnData.length() > 0) {
        int givenAddr = 0;
        int slaveUnique = returnData.substring(2, 3).toInt();
        int slaveId = returnData.substring(4, 8).toInt();

        Serial.println("---");
        Serial.println(i);
        Serial.println(returnData);

        if (slaveUnique && findNearZero() < i) { //the addr is unique and has available addr before current one
          givenAddr = findNearZero();
        } else if (!slaveUnique) { //the address isn't unique
          givenAddr = UNSET_ADDR;
          allUni = false;
        }

        if (givenAddr == 0) { //adress is already determined
          if (modules[i][0] == 0) { //slave has no id
            modules[i][0] = generateUniqueID(); //generate an identity

            //modules[i][0] = 1234;
            sendCmd(i, "3" + String(modules[i][0])); //update device's address
          } else {

            if (modules[i][0] != slaveId) {
              Serial.println("conflict id");
              sendCmd(i, "4" + String(modules[i][0])); //tell to rearrange address by checking id (to tell this address's devices to self rearrange, only the slave with the id can have the address)
            } else {
              int slaveState = returnData.substring(3, 4).toInt();
              int slaveCurrent = returnData.substring(8, 12).toInt();

              //update the rest of information
              modules[i][1] = slaveState; //switchState update

              if (slaveCurrent - modules[i][2] - 10 > 0) lastPlugAddr = i; //current rises in a big ratio

              modules[i][2] = slaveCurrent; //current update
              newSysCurrent += slaveCurrent;
            }
          }
        } else if (givenAddr != UNSET_ADDR) { //update addr to other addr
          modules[givenAddr][0] = modules[i][0]; //move id to new addr
          modules[i][0] = 0;
        }

        if (givenAddr != 0) sendCmd(i, "2" + int2str(givenAddr, 2)); //update device's address
      } else {
        modules[i][0] = 0;
      }

      delay(2);
    }
  } while (!allUni);

  sysCurrent = newSysCurrent;
  }

  int scanSlaves() {
  return scanSlaves(false);
  }

  int scanSlaves(bool info) {
  uint8_t addr;
  int connc = 0;

  for (addr = 1; addr <= UNSET_ADDR; addr++) {
    i2cWire.beginTransmission(addr);

    if (i2cWire.endTransmission() == 0) {
      if (info) Serial.println(addr);

      connc++;
    } else {
      modules[addr][0] = 0;
    }
  }

  return connc;
  }
*/
