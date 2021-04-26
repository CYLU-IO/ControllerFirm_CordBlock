/***
 * Properties
 */
 
/***
 * Basic Functions
 */
void sensInit() {
  pinMode(conncLedPin, OUTPUT);
  pinMode(powerLedPin, OUTPUT);
  
  digitalWrite(conncLedPin, LOW);
  digitalWrite(powerLedPin, HIGH);
}

void sensLoop() {

}
