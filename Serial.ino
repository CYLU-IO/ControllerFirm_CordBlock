void serialInit() {
  Serial.begin(9600);
  Serial1.begin(9600);
}

void serialEnd() {
  Serial.end();
  Serial1.end();
}

void clearSerial1() {
  while (Serial1.available() > 0) Serial1.read();
}

void serialSignalProcess() {
  if (Serial1.available() > 0) {
    char sig = char(Serial1.read());
    
    if (sig == 'A') {
      StaticJsonDocument<32> data;

      data["action"] = 1;
      data["addr"] = 0;

      serializeJson(data, Serial1);
      digitalWrite(conncLedPin, LOW);
    } else if (sig == 'C') {
      String m = "";

      while (true) {
        if (Serial1.available() > 0) {
          char c = char(Serial1.read());

          if (c == 'D') break;

          m += c;
        }
      }

      StaticJsonDocument<512> data;
      DeserializationError err = deserializeJson(data, m);

      if (err == DeserializationError::Ok) {
        int updateNumModule = data["amount"].as<int>();

        if (numModule != updateNumModule) Serial.println(Homekit.init((const char*)serial_num, acc_name, acc_code, acc_setupId)); //Homekit

        for (int i = updateNumModule - 1; i >= 0; i--) {
          modules[i][0] = data["modules"][i][0].as<int>(); //insert id into slaves table
          modules[i][1] = data["modules"][i][1].as<int>(); //insert switch state into slaves table
          modules[i][2] = data["modules"][i][2].as<int>(); //insert current into slaves table
          Serial.print("Addr: ");
          Serial.println(i);
          Serial.println(modules[i][0]);
          Serial.println(data["modules"][i][3].as<const char*>());

          Serial.println(Homekit.updateService((uint8_t)i + 1, (uint8_t)modules[i][0], (uint8_t)modules[i][1], data["modules"][i][3].as<const char*>()));
          Serial.println("...");
        }

        if (numModule != updateNumModule) Serial.println(Homekit.begin());

        numModule = updateNumModule;
        Serial.println(numModule);
        Serial.println("-----");

        boardcastWork();
        digitalWrite(conncLedPin, HIGH); //finish connection
      } else {
        Serial.println(err.c_str());
        clearSerial1();
      }
    }
  }
}

void getModuleConnection() {
  if (numModule == 0) { //no module recorded
    return;
  }
}
