void currentChecker() {
  if (sysCurrent > MAX_CURRENT) { //check if system current is over loaded
    if (advSmtOverloader) {
      //check order
    } else {
      Serial.println("System current is overloaded! Cut down the last-plugged.");
      //if (lastPlugAddr != 0) sendCmd(lastPlugAddr, 5); //cut the overloaded itself
    }
  }
}
