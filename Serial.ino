StaticJsonDocument<96> data;

void serialInit() {
  pinPeripheral(5, PIO_SERCOM_ALT);
  pinPeripheral(6, PIO_SERCOM_ALT);

#if DEBUG
  Serial.begin(9600);
#endif

  Serial1.begin(9600);

  /**
     Serial 3 is used for boardcasting CMD to modules.
     It is not allowed to receive CMD.
  */
  Serial3.begin(9600); //RX: 5, TX: 6
}


void moduleReconncTrial() {
  sendAddress(Serial1);
}


void receiveSerial() {
  static CMD_STATE state = RC_NONE;

  static char cmd;
  static int length;
  static int buffer_pos;
  static char buffer[96];

  if (Serial1.available()) {
    char c = receiveCmd(Serial1, state, cmd, length, buffer_pos, buffer);

    switch (c) {
      case CMD_REQ_ADR: {
          sendAddress(Serial1);
          break;
        }

      case CMD_LINK_MODULE: {
          DeserializationError err = deserializeJson(data, buffer);

          if (err == DeserializationError::Ok) {
            int updateNumModule = data["total"].as<int>();
            int index = data["addr"].as<int>() - 1;
            const char* name = data["name"].as<const char*>();

            digitalWrite(MODULES_STATE_PIN, LOW);

            if (index + 1 == updateNumModule) { //first module arrives
              sys_status.module_initialized = false;

#if ENABLE_HOMEKIT
              if (Homekit.countAccessory() > 0) {
                Serial.print(F("[HOMEKIT] Delete previous accessory: "));
                Serial.println(Homekit.deleteAccessory());
              }

              Serial.print(F("[HOMEKIT] Create accessory: "));
              Serial.println(Homekit.createAccessory((const char*)acc_info.serial_number, (const char*)acc_info.name));
#endif
            }

            sys_info.modules[index][0] = data["pri"].as<int>(); //insert id into slaves table
            sys_info.modules[index][1] = data["switch_state"].as<int>(); //insert switch state into slaves table
            sys_info.modules[index][2] = 0;

#if DEBUG
            Serial.print("Addr: "); Serial.println(index + 1);
            Serial.print("Priority: "); Serial.println(sys_info.modules[index][0]);
            Serial.print("Name: "); Serial.println(name);
#endif

#if ENABLE_HOMEKIT
            Serial.print(F("[HOMEKIT] Add service: "));
            Serial.println(Homekit.addService(index, sys_info.modules[index][1], name));
#endif

            if (index == 0) {
              sys_info.num_modules = updateNumModule;

              Serial.print(F("[UART] Total modules: ")); Serial.println(updateNumModule);

              char *p = (char*)malloc(updateNumModule * sizeof(char));
              for (int i = 0; i < updateNumModule; i++) p[i] = i + 1;
              sendCmd(Serial3, CMD_INIT_MODULE, p, updateNumModule); //using boardcast

              sendReqData(Serial3, MODULE_CURRENT);

#if ENABLE_HOMEKIT
              Serial.print(F("[HOMEKIT] Begin HAP service: ")); Serial.println(Homekit.beginAccessory());
#endif

              digitalWrite(MODULES_STATE_PIN, HIGH);
              sys_status.module_initialized = true;
              Serial.println("[UART] Connection done");
            }
          } else {
            sendAddress(Serial1);
          }
          break;
        }

      case CMD_UPDATE_DATA: {
          if (length < 3) return;

          int addr = buffer[0];
          int value = bytesCombine(buffer[2], buffer[3]);

          switch (buffer[1]) {
            case MODULE_SWITCH_STATE: {
#if DEBUG
                Serial.print("[UART] Module ");
                Serial.print(addr);
                Serial.print(" state changes to ");
                Serial.println(value);
#endif
                sys_info.modules[addr - 1][1] = value;
                Homekit.setServiceValue(addr - 1, value); //set homekit state forcibly
                break;
              }

            case MODULE_CURRENT: {
#if DEBUG
                Serial.print("[UART] Module ");
                Serial.print(addr);
                Serial.print(" current updates to ");
                Serial.println(value);
#endif

                /*** Check MCUB Triggering ***/
                if (value >= sys_info.modules[addr - 1][2] + smf_info.mcub) {
                  smf_info.overload_triggered_addr = addr;
                  smartCurrentCheck();

#if DEBUG
                  Serial.print("[SMF] MCUB Triggered by module ");
                  Serial.println(addr);
#endif
                }

                /*** Update module current data ***/
                sys_info.modules[addr - 1][2] = value;

                /*** Update MCUB ***/
                int sum = 0;
                for (int i = 0; i < sys_info.num_modules; i++) sum += sys_info.modules[i][2];
                sys_info.sum_current = sum;

                int mcub = (MAX_CURRENT - sum) / sys_info.num_modules;
                mcub = (mcub >= 0) ? mcub : 0;
                smf_info.mcub = mcub;

                int a[1] = {0};
                int v[1] = {mcub};
                sendUpdateData(Serial3, MODULE_MCUB, a, v, 1);

#if DEBUG
                Serial.print("[SMF] Update MCUB: ");
                Serial.println(smf_info.mcub);
#endif
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

#if DEBUG
  Serial.println("[UART] Sending address");
#endif
}

void sendDoModule(Stream &_serial, char act, char* target, int length) {
  int l = 1 + length;
  char *p = (char*)malloc(l * sizeof(char));

  p[0] = act;

  for (int i = 1; i < l; i++) p[i] = target[i - 1];

  sendCmd(_serial, CMD_DO_MODULE, p, l);
  free(p);
}

void sendReqData(Stream &_serial, char type) {
  char p[1] = {type};
  sendCmd(_serial, CMD_REQ_DATA, p, sizeof(p));
}

void sendUpdateData(Stream &_serial, char type, int* addr, int* value, int length) {
  int l = length * 3 + 1; //a pack takes three bytes
  char *p = (char*)malloc(l * sizeof(char));

  p[0] = type;

  for (int i = 0; i < length; i++) {
    p[i * 3 + 1] = addr[i];
    p[i * 3 + 2] = value[i] & 0xff;
    p[i * 3 + 3] = (value[i] >> 8) & 0xff;
  }

  sendCmd(_serial, CMD_UPDATE_DATA, p, l);
  free(p);
}

/*** Util ***/
char receiveCmd(Stream &_serial, CMD_STATE &state, char &cmd, int &length, int &buffer_pos, char *buffer) {
  Stream* serial = &_serial;

  switch (state) {
    case RC_NONE: {
        cmd = CMD_FAIL;
        if (serial->available() < 1) break;

        uint8_t start_byte = serial->read();

        if (start_byte != CMD_START) {
#if DEBUG
          Serial.print("[UART] Incorrect start byte: ");
          Serial.println(start_byte, HEX);
#endif
          break;
        }

        state = RC_HEADER;
      }

    case RC_HEADER: {
        if (serial->available() < 3) break;

        cmd = serial->read();

        length = serial->read();
        length |= (uint16_t) serial->read() << 8;

        buffer_pos = 0;

        state = RC_PAYLOAD;
      }

    case RC_PAYLOAD: {
        if (buffer_pos < length && serial->available()) {
          buffer[buffer_pos++] = serial->read();
        }

        if (buffer_pos < length) break;

        state = RC_CHECK;
      }

    case RC_CHECK: {
        if (serial->available() < 2) break;

        uint8_t checksum = serial->read();
        uint8_t eof = serial->read();

        state = RC_NONE;

        if (eof != CMD_EOF) {
#if DEBUG
          Serial.print("[UART] ERROR: Unexpected EOF: ");
          Serial.println(eof, HEX);
#endif

          break;
        }

        return cmd;
      }
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

void clearSerial(Stream &_serial) {
  Stream* serial = &_serial;

  while (serial->available() > 0) serial->read();
}
