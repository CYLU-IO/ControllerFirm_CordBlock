StaticJsonDocument<96> data;
int cmdLength;
char cmdBuf[96];

void serialInit() {
  Serial.begin(9600);
  Serial1.begin(9600);
}


void moduleReconncTrial() {
  if (Serial1.available() > 0) return; //Slave may not be initialized

  sendAddress(Serial1);
}


void receiveSerial() {
  if (Serial1.available()) {
    char cmd = receiveCmd(Serial1);

    switch (cmd) {
      case CMD_REQ_ADR: {
          sendAddress(Serial1);
          break;
        }

      case CMD_LINK_MODULE: {
          DeserializationError err = deserializeJson(data, cmdBuf);

          if (err == DeserializationError::Ok) {
            int updateNumModule = data["total"].as<int>();
            int index = data["addr"].as<int>() - 1;
            const char* name = data["name"].as<const char*>();

            if (index + 1 == updateNumModule) { //first module arrives
              sys_status.module_initialized = false;
              digitalWrite(MODULES_CONNC_STATE_PIN, LOW);

              if (sys_info.num_modules > 0) {
                Serial.print(F("[HOMEKIT] Delete previous accessory: "));
                Serial.println(Homekit.deleateAccessory());
              }

              Serial.print(F("[HOMEKIT] Create accessory: "));
              Serial.println(Homekit.create((const char*)acc_info.serial_number, (const char*)acc_info.name));
            }

            sys_info.modules[index][0] = data["id"].as<int>(); //insert id into slaves table
            sys_info.modules[index][1] = data["switch_state"].as<int>(); //insert switch state into slaves table
            sys_info.modules[index][2] = 0;

            Serial.print("Addr: "); Serial.println(index + 1);
            Serial.print("ID: "); Serial.println(sys_info.modules[index][0]);
            Serial.print("Name: "); Serial.println(name);

            if (index == 0) {
              sys_info.num_modules = updateNumModule;
              
              for (int i = 0; i < updateNumModule; i++) {
                Serial.print(F("[HOMEKIT] Add service: "));
                Serial.println(Homekit.addService(i,
                                                  sys_info.modules[i][0],
                                                  sys_info.modules[i][1],
                                                  name));
              }

              Serial.print(F("[HOMEKIT] Begin HAP service: ")); Serial.println(Homekit.begin());
              Serial.print(F("[UART] Total modules: ")); Serial.println(updateNumModule);

              char *p = (char*)malloc(updateNumModule * sizeof(char));
              for (int i = 0; i < updateNumModule; i++) p[i] = i + 1;
              sendCmd(Serial1, CMD_INIT_MODULE, p, updateNumModule); //pass to tell modules start I2C service
              
              digitalWrite(MODULES_CONNC_STATE_PIN, HIGH);
              sys_status.module_initialized = true;
              Serial.println("[UART] Connection done");
            }
          } else {
            sendAddress(Serial1);
          }
          break;
        }

      case CMD_UPDATE_MASTER: {
          if (cmdLength < 3) return;
          
          int addr = cmdBuf[0];
          int value = cmdBuf[2];

          switch (cmdBuf[1]) {
            case MODULE_SWITCH_STATE: {
                Serial.print("[UART] Module state changes to "); Serial.println(value);
                sys_info.modules[addr - 1][1] = value;
                break;
              }

            case MODULE_CURRENT: {
                Serial.print("[UART] Module current changes to "); Serial.println(value);
                sys_info.modules[addr - 1][2] = value;
                break;
              }
          }

          break;
        }
    }
  }
}

void sendAddress(Stream &_serial) {
  Stream* serial = &_serial;

  char p[1] = {0};
  clearSerial(Serial1);
  sendCmd(_serial, CMD_LOAD_MODULE, p, sizeof(p));
  serial->println("Sending address");
}

void sendDoModule(Stream &_serial, char act, char* target, int length) {
  int l = 1 + length;
  char *p = (char*)malloc(l * sizeof(char));

  p[0] = act;

  for (int i = 1; i < l; i++) p[i] = target[i - 1];

  sendCmd(_serial, CMD_DO_MODULE, p, l);
  free(p);
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

    uint8_t checksum = serialRead(_serial); //checksum

    if (serialRead(_serial) != CMD_EOF && calcCRC(cmdBuf, cmdLength) != checksum) return CMD_FAIL; //error

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
  buf[4 + length] = calcCRC(payload, length); //checksum
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
