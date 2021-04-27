void  mqttInit() {
  client.setServer(MQTT_SERVER_IP, 1883);

  while (!client.connected()) {
    mqttConnect();
    client.setCallback(mqttListerner);
  }
}

void mqttConnect() {
  Serial.print("Attempting MQTT connection...");

  String mqttrId = MQTT_CLIENT_ID + int2str(generateUniqueID(), 4);
  char * buf = (char *) malloc (mqttrId.length());
  mqttrId.toCharArray(buf, mqttrId.length() + 1);

  if (client.connect(buf)) {
    Serial.println("connected");
    client.subscribe(MQTT_SUB_TOPIC);
    free(buf);
  } else {
    Serial.print("failed, rc=");
    Serial.print(client.state());
    Serial.println(" try again in 5 seconds");

    delay(5000);
  }
}

void mqttListerner(char* topic, byte* payload, unsigned int length) {
  String m = "";

  for (int i = 0; i < length; i ++) m += char(payload[i]);

  StaticJsonDocument<256> json;
  deserializeJson(json, payload);

  for (int i = 0; i < json.size(); i++) {
    int id = json[i]["id"];
    int type = json[i]["type"];
    int state = json[i]["state"];

    Serial.println("Received from " + int2str(id, 4));
    Serial.println("State:" + int2str(state, 1));

    if (type == 1) { //meaning "action"
      switch (state) {
        case 1:
          Serial.print("| turn ON");

          break;

        case 2:

          Serial.print("| turn OFF");
          break;
      }
    }
  }
}

void mqttLoop() {
  /*while (wifiStatus != WL_CONNECTED) {
    wifiConncInit();
    }
    /*StaticJsonDocument<256> json;
    char buf[256];


    for (int i = 1; i < UNSET_ADDR; i++) {
    if (slaves[i][0] != 0) {
      JsonArray slaveData = json.createNestedArray(int2str(slaves[i][0], 4));
      JsonObject data = slaveData.createNestedObject();

      data["state"] = slaves[i][1];
      data["priority"] = slaves[i][2];
      data["current"] = slaves[i][3];
    }
    }

    serializeJson(json, buf);
    //client.publish(MQTT_SUB_TOPIC, buf);
    //Serial.println(buf);*/
}
