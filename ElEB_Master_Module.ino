#include <SPI.h>
#include <Wire.h>
#include <WiFiNINA.h>
#include <ArduinoBLE.h>

#define MAX_CURRENT 500

/***
   Global Parameters
*/
String wifiType, wifiSSID, wifiUser, wifiPass;

bool advSmtOverloader = false;
int sysCurrent = 0;
int lastPlugAddr = 0;

/***
   Pin Settings
*/
int conncLedPin = 9;
int powerLedPin = 7;

void setup() {
  Serial.begin(9600);

  sensInit();

  wifiSSID = "Edwin's Room";
  wifiPass = "Edw23190";
  wifiConncInit();
  
  i2cManInit();
  
}

void loop() {
  sensLoop();
  i2cManLoop();
  //bleConncLoop();

  delay(10);
}
