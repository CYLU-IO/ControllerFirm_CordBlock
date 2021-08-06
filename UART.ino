int uartReceive(Stream &_serial, UART_MSG_RC_STATE &state, int &length, char *buffer, int &buffer_pos) {
  Stream* serial = &_serial;

  switch (state) {
    case RC_NONE: {
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
        if (serial->available() < 2) break;

        length = serial->read();
        length |= (uint16_t) serial->read() << 8;

        buffer_pos = 0;

        state = RC_PAYLOAD;
      }

    case RC_PAYLOAD: {
        while (buffer_pos < length && serial->available()) {
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

        if (checksum != calcCRC(buffer, length)) {
#if DEBUG
          Serial.print("[UART] ERROR: Incorrect CRC8: ");
          Serial.println(checksum, HEX);
          break;
#endif
        }

        if (eof != CMD_EOF) {
#if DEBUG
          Serial.print("[UART] ERROR: Unexpected EOF: ");
          Serial.println(eof, HEX);
#endif

          break;
        }

        return 1; //OK
      }
  }

  return 0; //INCOMPLETE
}

void uartTransmit(Stream &_serial, int length, char* payload) {
  Stream* serial = &_serial;
  char crc = calcCRC(payload, length);

  serial->write(CMD_START);
  serial->write(length & 0xff);
  serial->write(length >> 8 & 0xff);

  for (int i = 0; i < length; i++) serial->write(payload[i]);

  serial->write(crc);
  serial->write(CMD_EOF);

  free(payload);
}
