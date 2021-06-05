uint8_t calcCRC(char* str) {
  CRC32 crc;
  for (int i = 0; i < strlen(str); i++) crc.update(str[i]);

  return crc.finalize();
}

int findNearZero() {
  for (int i = 1; i < sizeof(sys_info.modules); i++) if (sys_info.modules[i][0] == 0) return i;

  return -1;
}

int searchAddrById(int id) {
  for (int i = 1; i < sys_info.num_modules; i++) if (sys_info.modules[i][0] == id) return i;

  return 0;
}

/***
 * Format
 */
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

char randChar() {
  byte randomValue = random(0, 37);
  char letter = randomValue + 'A';

  if (randomValue > 26) letter = (randomValue - 26) + '0';

  return letter;
}

void serialNumGenerator(char* sn, const char* manufacturer, const char* year, const char* week, int id_len) {
  strcpy(sn, manufacturer);
  strcat(sn, year);
  strcat(sn, week);

  for (int i = sizeof(sn) + 1; i < id_len + sizeof(sn); i++) sn[i] = randChar();
}
