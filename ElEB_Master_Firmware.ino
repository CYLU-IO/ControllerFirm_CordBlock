#include <SPI.h>
#include <Wire.h>
#include <ArduinoBLE.h>
#include <WiFiNINA.h>

/***
   Global Parameters
*/
String wifiType, wifiSSID, wifiUser, wifiPass;
int sysCurrent = 0;

/***
 * Pin Settings
 */
int conncLedPin = 13;

void setup() {
  Serial.begin(9600);

  sensInit();
  i2cManInit();
  bleConncInit();
}

void loop() {
  sensLoop();
  i2cManLoop();
  bleConncLoop();

  delay(20);
}
