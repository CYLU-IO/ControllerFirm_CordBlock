void wifiInit() {
  if (WiFi.status() != WL_NO_MODULE) {
    int wifiStatus = WL_IDLE_STATUS;

    WiFi.lowPowerMode();
    Serial.print(F("[WiFi] Attempting to connect..."));

    while (wifiStatus != WL_CONNECTED) {
      wifiStatus = WiFi.begin(wifi_setting.ssid, wifi_setting.password);
      delay(2000);
    }

    digitalWrite(WIFI_STATE_PIN, HIGH);
    Serial.print(F("OK\n"));
  } else {
    Serial.print(F("failed\n"));
  }
}

bool isWifiConncInfoFilled() {
  return (sizeof(wifi_setting.ssid) * sizeof(wifi_setting.password) > 0) ? true : false;
}

bool checkWiFiConnc() {
  bool _running =  (WiFi.status() == 3) ? HIGH : LOW;

  digitalWrite(WIFI_STATE_PIN, _running);

  return _running;
}
