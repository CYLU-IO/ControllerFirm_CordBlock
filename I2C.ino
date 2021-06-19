TwoWire i2cWire(&sercom1, 11, 13);

void i2cInit() {
  i2cWire.begin();

  pinPeripheral(11, PIO_SERCOM);
  pinPeripheral(13, PIO_SERCOM);
}

void sendI2CCmd(TwoWire &_wire, char cmd, char* payload, int length) {
  TwoWire* wire = &_wire;
  char buf[5 + length]; //start, data_length(uint8), data, checksum, stop

  buf[0] = CMD_START; //start_byte
  buf[1] = cmd; //cmd_byte
  buf[2] = length & 0xff; //data_length
  buf[3 + length] = calcCRC(payload, length); //checksum
  buf[4 + length] = CMD_EOF; //stop_byte

  for (int i = 0; i < length; i++) //load buf
    buf[3 + i] = payload[i];
    
  wire->beginTransmission(0x01);
  wire->write(buf);
  wire->endTransmission();
}

void sendI2CCmd(char cmd, char* payload, int length) {
  sendI2CCmd(i2cWire, cmd, payload, length);
}

void sendDoModule(char act, char* target, int length) {
  int l = 1 + length;
  char *p = (char*)malloc(l * sizeof(char));

  p[0] = act;

  for (int i = 1; i < l; i++) p[i] = target[i - 1];

  sendI2CCmd(CMD_DO_MODULE, p, l);
  free(p);
}
