int findNearZero() {
  for (int i = 1; i < 51; i++) if (modules[i][0] == 0) return i;

  return -1;
}

int searchAddrById(int id) {
  for (int i = 1; i < UNSET_ADDR; i++) {
    if (modules[i][0] == id) return i;
  }

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
