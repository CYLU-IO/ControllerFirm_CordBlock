/***
   Definitions
*/
#define UNSET_ADDR 51

/***
   Parameters
*/
int slaves[51][3] = {0}; //slave address[id][switchState][current]

/***
   Basic Functions
*/
void i2cInit() {
  i2cWire.begin();

  pinPeripheral(11, PIO_SERCOM);
  pinPeripheral(13, PIO_SERCOM);
}

void i2cLoop() {
  initSlaves();
}

/*
   I2C Utilites
*/
void initSlaves() {
  bool allUni = true;
  int newSysCurrent;

  do {
    allUni = true;

    int nearZero = findNearZero();
    newSysCurrent = 0;
    sendCmd(UNSET_ADDR, "1" + int2str(findNearZero() + 1, 2) + "50"); //announce to addressing randomly

    for (int i = 1; i <= UNSET_ADDR; i++) {
      String returnData = receiveMessage(i, 12); //request data from slaves
      
      if (returnData.length() > 0) {
        int givenAddr = 0;
        int slaveUnique = returnData.substring(2, 3).toInt();
        int slaveId = returnData.substring(4, 8).toInt();

        if (slaveUnique && findNearZero() < i) { //the addr is unique and has available addr before current one
          givenAddr = findNearZero();
        } else if (!slaveUnique) { //the address isn't unique
          givenAddr = UNSET_ADDR;
          allUni = false;
        }

        if (givenAddr == 0) { //adress is already determined
          if (slaves[i][0] == 0) { //slave has no id
            //slaves[i][0] = generateUniqueID(); //generate an identity

            slaves[i][0] = 1234;
            sendCmd(i, "3" + String(slaves[i][0])); //update device's address
          } else {
            if (slaves[i][0] != slaveId) {
              sendCmd(i, "4" + String(slaves[i][0])); //tell to rearrange address by checking id (to tell this address's devices to self rearrange, only the slave with the id can have the address)
            } else {
              int slaveState = returnData.substring(3, 4).toInt();
              int slaveCurrent = returnData.substring(8, 12).toInt();

              //update the rest of information
              slaves[i][1] = slaveState; //switchState update

              if (slaveCurrent - slaves[i][3] - 10 > 0) lastPlugAddr = i; //current rises in a big ratio

              slaves[i][3] = slaveCurrent; //current update
              newSysCurrent += slaveCurrent;
            }
          }
        } else if (givenAddr != UNSET_ADDR) { //update addr to other addr
          slaves[givenAddr][0] = slaves[i][0]; //move id to new addr
          slaves[i][0] = 0;
        }

        if (givenAddr != 0) sendCmd(i, "2" + int2str(givenAddr, 2)); //update device's address
      } else {
        slaves[i][0] = 0;
      }
    }
  } while (!allUni);

  sysCurrent = newSysCurrent;
}

String receiveMessage(int addr, int mlength) {
  String response = "";

  i2cWire.requestFrom(addr, mlength);

  while (i2cWire.available()) {
    char b = i2cWire.read();
    response += b;
  }

  return response;
}

int sendCmd(int addr, String rawCmd) {
  char * buf = (char *) malloc (rawCmd.length());
  rawCmd.toCharArray(buf, rawCmd.length() + 1);

  i2cWire.beginTransmission(addr);
  i2cWire.write(buf);
  free(buf);

  return i2cWire.endTransmission();
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
      slaves[addr][0] = 0;
    }
  }

  return connc;
}
