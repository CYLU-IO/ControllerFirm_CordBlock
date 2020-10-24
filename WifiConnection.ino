/***
   Properties
*/
int wifiStatus = WL_IDLE_STATUS;

/***
   Basic Functions
*/
void wifiConncInit() {
  BLE.end();

  if (WiFi.status() != WL_NO_MODULE) {
    Serial.println("Starting Wifi Connection...");
    
    while (wifiStatus != WL_CONNECTED) {
      wifiStatus = WiFi.begin(wifiSSID.c_str(), wifiPass.c_str());
      delay(5000);
    }

    Serial.println("Successfully Connecting Wifi");
    digitalWrite(conncLedPin, HIGH);
  }
}

bool isWifiConncInfoFilled() {
  return (wifiSSID.length()*wifiPass.length() > 0) ? true : false;
}
