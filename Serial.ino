void serialInit() {
  Serial.begin(9600);
  Serial1.begin(9600);
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
        connectedSlave = data["amount"].as<int>();
        Serial.println(connectedSlave);
        Serial.println("-----");
        
        for (int i = connectedSlave - 1; i >= 0; i--) {
          modules[i][0] = data["id"][i].as<int>(); //insert id into slaves table
          Serial.println(modules[i][0]);
        }

        digitalWrite(conncLedPin, HIGH); //finish connection
      } else {
        Serial.println(err.c_str());
        clearSerial1();
      }
    }
  }
}
