#define CMD_OK              0x10
#define CMD_FAIL            0x11
#define CMD_PASS_MSG        0x12
#define CMD_INSYNC          0x14
#define CMD_NOSYNC          0x15
#define CMD_EOF             0x20
#define CMD_REQ_ADR         0x41 //'A'
#define CMD_LOAD_MODULE     0x42 //'B'
#define CMD_CONFIRM_RECEIVE 0x43 //'C'
#define CMD_LINK_MODULE     0x4C //'L'
#define CMD_INIT_MODULE     0x49 //'I'
#define CMD_RELOAD_MODULE   0x52 //'R'
#define CMD_START           0xFF

StaticJsonDocument<96> data;
int cmdLength;
char cmdBuf[96];

void serialInit() {
  Serial.begin(9600);
  Serial1.begin(9600);
}


void moduleReconncTrial() {
  if (Serial1.available() > 0) return; //Slave may not be initialized

  char p[1] = {0};
  sendCmd(Serial1, CMD_LOAD_MODULE, p, sizeof(p));
}


void receiveSerial() {
  if (Serial1.available()) {
    char cmd = receiveCmd(Serial1);

    switch (cmd) {
      case CMD_REQ_ADR: {
          char p[1] = {0};
          clearSerial(Serial1);
          sendCmd(Serial1, CMD_LOAD_MODULE, p, sizeof(p));
          Serial.println("Sending address");

          break;
        }

      case CMD_LINK_MODULE: {
          DeserializationError err = deserializeJson(data, cmdBuf);

          if (err == DeserializationError::Ok) {
            int updateNumModule = data["total"].as<int>();
            int index = data["addr"].as<int>() - 1;
            const char* name = data["name"].as<const char*>();

            if (index + 1 == updateNumModule) {
              Serial.print("Total: ");
              Serial.println(updateNumModule);
              if (sys_info.num_modules > 0) {
                Serial.print(F("[Homekit] Delete previous accessory: "));
                Serial.println(Homekit.deleateAccessory());
              }

              Serial.print(F("[Homekit] Create accessory: "));
              Serial.println(Homekit.create((const char*)acc_info.serial_number, (const char*)acc_info.name));
            }

            sys_info.modules[index][0] = data["id"].as<int>(); //insert id into slaves table
            sys_info.modules[index][1] = data["switch_state"].as<int>(); //insert switch state into slaves table
            sys_info.modules[index][2] = 0;

            Serial.print("Addr: ");
            Serial.println(index + 1);

            Serial.print("ID: ");
            Serial.println(sys_info.modules[index][0]);

            Serial.print("Name: ");
            Serial.println(name);

            Serial.print(F("[Homekit] Add service: "));
            Serial.println(Homekit.addService(index,
                                              sys_info.modules[index][0],
                                              sys_info.modules[index][1],
                                              name));

            sendCmd(Serial1, CMD_CONFIRM_RECEIVE);

            if (index == 0) {
              Serial.println("Done");
              Serial.print(F("[Homekit] Begin HAP service: "));
              Serial.println(Homekit.begin());

              sys_info.num_modules = updateNumModule;
              sendCmd(Serial1, CMD_INIT_MODULE); //pass to tell modules start I2C service
              digitalWrite(MODULES_CONNC_STATE_PIN, HIGH); //finish connection
              //clearSerial(Serial1);
            }
          } else {
            Serial.println(err.c_str());
          }
          break;
        }
    }
  }
}


/*** Util ***/
char receiveCmd(Stream &_serial) {
  Stream* serial = &_serial;
  uint8_t sb = serialRead(_serial);

  if (sb == CMD_START) {
    char cmd = serialRead(_serial); //cmd_byte

    uint16_t length = serialRead(_serial);
    cmdLength = length | serialRead(_serial) << 8;

    if (cmdLength > sizeof(cmdBuf)) { //oversize
      eraseCmdBuf();
      while (serialRead(_serial) != CMD_EOF);
      return CMD_FAIL;
    }

    int buf_count = 0;
    if (length > 0) eraseCmdBuf();

    while (buf_count != cmdLength) {
      cmdBuf[buf_count] = serialRead(_serial);
      buf_count++;
    }

    char checksum = serialRead(_serial); //checksum, don't bother it yett

    if (serialRead(_serial) != CMD_EOF) return CMD_FAIL; //error

    return cmd;
  }

  return CMD_FAIL;
}

void sendCmd(Stream &_serial, char cmd, char* payload, int length) {
  Stream* serial = &_serial;
  char buf[6 + length]; //start, data_length, data, checksum, stop

  buf[0] = CMD_START; //start_byte
  buf[1] = cmd; //cmd_byte
  buf[2] = length & 0xff; //data_length - low byte
  buf[3] = (length >> 8) & 0xff; //data_length - high byte
  buf[4 + length] = (char)calcCRC(payload); //checksum
  buf[5 + length] = CMD_EOF; //stop_byte

  for (int i = 0; i < length; i++) //load buf
    buf[4 + i] = payload[i];

  for (int i = 0; i < sizeof(buf); i++)
    serial->print(buf[i]);
}
void sendCmd(Stream &_serial, char cmd) {
  char *p = 0x00;
  sendCmd(_serial, cmd, p, 0);
}

char serialRead(Stream &_serial) {
  Stream* serial = &_serial;

  while (!serial->available())
    delay(1);

  return (uint8_t)serial->read();
}

void eraseCmdBuf() {
  for (int i = 0; i < sizeof(cmdBuf); i++) cmdBuf[i] = 0x00;
}

void clearSerial(Stream &_serial) {
  Stream* serial = &_serial;

  while (serial->available() > 0) serial->read();
}
