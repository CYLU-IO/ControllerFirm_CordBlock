/***
   Definitions
*/
#define UNSET_ADDR 51

/***
   Parameters
*/
int slaves[51][4] = {0}; //slave address[id][switchState][priority][current]

/***
   Basic Functions
*/
void i2cManInit() {
  Wire.begin();
}

void i2cManLoop() {
  initSlaves();
  delay(400);
}

/*
   I2C Utilites
*/
void initSlaves() {
  bool allUni = true;
  int newSysCurrent;

  do {
    int nearZero = findNearZero();
    newSysCurrent = 0;

    Serial.println("---");
    scanSlaves();
    sendCmd(UNSET_ADDR, "1" + int2str(findNearZero() + 1, 2) + "50"); //announce to addressing randomly

    for (int i = 1; i < UNSET_ADDR; i++) {
      String returnData = receiveMessage(i, 9);

      if (returnData.length() > 0) {
        int givenAddr = 0;
        int reportCurrent = returnData.substring(5).toInt();

        //Serial.println(returnData);
        newSysCurrent += reportCurrent;

        if (returnData.substring(2, 3).toInt() && findNearZero() < i) { //when addr is determined
          givenAddr = findNearZero();
        } else if (!returnData.substring(2, 3).toInt()) { //the address isn't unique
          givenAddr = UNSET_ADDR;
          allUni = false;
        }

        if (givenAddr == 0) { //adress is already determined
          if (slaves[i][0] == 0) { //slave has no id
            Serial.println(slaves[i][0]);
            //slaves[i][0] = generateUniqueID(); //generate an identity
            sendCmd(i, "3" + String(slaves[i][0])); //update device's address
          }

          slaves[i][3] = reportCurrent; //current update
        }

        if (givenAddr != 0) sendCmd(i, "2" + int2str(givenAddr, 2)); //update device's address
      }
    }

    delay(500);
  } while (!allUni);

  sysCurrent = newSysCurrent;
}

String receiveMessage(int addr, int mlength) {
  String response = "";

  Wire.requestFrom(addr, mlength);

  while (Wire.available()) {
    char b = Wire.read();
    response += b;
  }

  return response;
}

int sendCmd(int addr, String rawCmd) {
  char * buf = (char *) malloc (rawCmd.length());
  rawCmd.toCharArray(buf, rawCmd.length() + 1);

  Wire.beginTransmission(addr);
  Wire.write(buf);
  free(buf);

  return Wire.endTransmission();
}

int scanSlaves() {
  return scanSlaves(false);
}

int scanSlaves(bool info) {
  uint8_t addr;
  int connc = 0;

  for (addr = 1; addr <= UNSET_ADDR; addr++) {
    Wire.beginTransmission(addr);

    if (Wire.endTransmission() == 0) {
      if (info) Serial.println(addr);

      connc++;
    }
  }

  return connc;
}


/***
   Utilites
*/
int generateUniqueID() {
  randomSeed(analogRead(0));

  int nID;

  while (true) {
    nID = random(1000, 9999);

    for (int i = 0; i < UNSET_ADDR; i++) {
      if (slaves[i][0] == nID) {
        nID = 0;
        break;
      }
    }

    if (nID != 0) break;
  }

  return nID;
}

int findNearZero() {
  for (int i = 1; i < 101; i++) if (slaves[i][0] == 0) return i;

  return -1;
}

String int2str(int n, int leng) {
  String re = "";

  for (int i = leng - 1; i > 0; i--) {
    if (n / (int)(pow(10, i) + ((n % 9 == 0) ? 1 : 0)) == 0) {
      re += "0";
      continue;
    }

    break;
  }

  return re + String(n);
}
