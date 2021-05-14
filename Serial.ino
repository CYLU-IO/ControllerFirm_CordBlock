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
      StaticJsonDocument<16> data;

      data["addr"] = 0;

      Serial1.print('C');
      serializeJson(data, Serial1);
      Serial1.print('D');
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
        Serial.println(m);
        int updateNumModule = data["amount"].as<int>();

        //if (numModule != updateNumModule) Serial.println(Homekit.init((const char*)acc_info.serial_number, (const char*)acc_info.name, (const char*)acc_info.code, (const char*)acc_info.setupId)); //Homekit

        for (int i = updateNumModule - 1; i >= 0; i--) {
          modules[i][0] = data["modules"][i][0].as<int>(); //insert id into slaves table
          modules[i][1] = data["modules"][i][1].as<int>(); //insert switch state into slaves table
          modules[i][2] = data["modules"][i][2].as<int>(); //insert current into slaves table
          Serial.print("Addr: ");
          Serial.println(i);
          Serial.println(modules[i][0]);
          Serial.println(data["modules"][i][3].as<const char*>());

          //Serial.println(Homekit.updateService((uint8_t)i + 1, (uint8_t)modules[i][0], (uint8_t)modules[i][1], data["modules"][i][3].as<const char*>()));
          Serial.println("...");
        }

        //if (numModule != updateNumModule) Serial.println(Homekit.begin());

        numModule = updateNumModule;
        Serial.println(numModule);
        Serial.println("-----");

        Serial1.print('I'); //pass to tell modules start I2C service
        //clearSerial1();
        digitalWrite(MODULES_CONNC_STATE_PIN, HIGH); //finish connection
      } else {
        Serial.println(err.c_str());
      }
    }
  }
}
