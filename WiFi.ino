void wifiConncInit() {
  int wifiStatus = WL_IDLE_STATUS;
  
  digitalWrite(conncLedPin, LOW);

  if (WiFi.status() != WL_NO_MODULE) {
    wifiStatus = WiFi.status();

    while (wifiStatus != WL_CONNECTED) {
      Serial.println(F("[WiFi] Attempting to connect WiFi"));
      WiFi.lowPowerMode();
      wifiStatus = WiFi.begin(wifiSSID.c_str(), wifiPass.c_str());
      
      delay(2000);
    }

    Serial.println(F("[WiFi] Successfully connect WiFi"));
    IPAddress ip = WiFi.localIP();
    Serial.print(F("[WiFi] IP Address: "));
    Serial.println(ip);

    digitalWrite(conncLedPin, HIGH);
  } else {
    Serial.println(F("[ERROR] Cannot connect WiFi"));
  }
}

bool isWifiConncInfoFilled() {
  return (wifiSSID.length() * wifiPass.length() > 0) ? true : false;
}
