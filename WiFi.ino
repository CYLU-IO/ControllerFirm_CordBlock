void wifiInit() {
  if (WiFi.status() != WL_NO_MODULE) {
    int wifiStatus = WL_IDLE_STATUS;

    WiFi.lowPowerMode();
    Serial.print(F("[WiFi] Attempting to connect..."));

    while (wifiStatus != WL_CONNECTED) {
      wifiStatus = WiFi.begin(wifiSSID.c_str(), wifiPass.c_str());
      delay(2000);
    }

    Serial.print(F("OK\n"));
  } else {
    Serial.print(F("failed\n"));
  }
}

bool isWifiConncInfoFilled() {
  return (wifiSSID.length() * wifiPass.length() > 0) ? true : false;
}

bool checkWiFi() {
  bool _running =  (WiFi.status() == 3) ? HIGH : LOW;

  digitalWrite(conncLedPin, _running);

  return _running;
}
