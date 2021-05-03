/***
 * Definitions
 */
#define MODEL_NUMBER "TLEB1"
#define FIRMWARE_VERSION "V1.0.0"
#define MANUFACTURER_NAME "Cylu Design"
#define DEVICE_NAME "Train-like E-Blocks"

/***
   BLE Services
*/
BLEService eBlocks("19B10000-E8F2-537E-4F6C-D104768A1214");
BLEService deviceInformation("180A");

/***
   Connection Info
*/
BLEStringCharacteristic wifiTypeCharacteristic("19B10001-E8F2-537E-4F6C-D104768A1214", BLERead | BLEWrite, 16);
BLEStringCharacteristic wifiSSIDCharacteristic("19B10002-E8F2-537E-4F6C-D104768A1214", BLERead | BLEWrite, 512);
BLEStringCharacteristic wifiUserCharacteristic("19B10003-E8F2-537E-4F6C-D104768A1214", BLERead | BLEWrite, 512);
BLEStringCharacteristic wifiPassCharacteristic("19B10004-E8F2-537E-4F6C-D104768A1214", BLERead | BLEWrite, 512);

/***
   System Info
*/
BLECharacteristic ModelNumberString("2A24", BLERead, MODEL_NUMBER);
BLECharacteristic FirmwareRevisionString("2A26", BLERead, FIRMWARE_VERSION);
BLECharacteristic ManufacturerNameString("2A29", BLERead, MANUFACTURER_NAME);

/***
   Parameters
*/
String conncLink = "192.168.1.1";

/***
   Basic Functions
*/
void bleConncInit() {
  if (BLE.begin()) {
    BLE.setDeviceName(DEVICE_NAME); //BLE boardcast name

    BLE.setAdvertisedService(eBlocks);
    BLE.setAdvertisedService(deviceInformation);

    eBlocks.addCharacteristic(wifiTypeCharacteristic);
    eBlocks.addCharacteristic(wifiSSIDCharacteristic);
    eBlocks.addCharacteristic(wifiUserCharacteristic);
    eBlocks.addCharacteristic(wifiPassCharacteristic);
    deviceInformation.addCharacteristic(ModelNumberString);
    deviceInformation.addCharacteristic(FirmwareRevisionString);
    deviceInformation.addCharacteristic(ManufacturerNameString);

    BLE.addService(eBlocks);
    BLE.addService(deviceInformation);

    wifiTypeCharacteristic.setEventHandler(BLEWritten, wifiTypeCharacteristicWritten);
    wifiSSIDCharacteristic.setEventHandler(BLEWritten, wifiSSIDCharacteristicWritten);
    wifiUserCharacteristic.setEventHandler(BLEWritten, wifiUserCharacteristicWritten);
    wifiPassCharacteristic.setEventHandler(BLEWritten, wifiPassCharacteristicWritten);

    BLE.advertise();
    Serial.println("Waiting for BLE Connection...");
  }
}

void bleConncLoop() {
  BLE.poll();
  BLEDevice central = BLE.central();

  if (central) {
    //
  }
}

/***
   Event Handlers
*/
void wifiTypeCharacteristicWritten(BLEDevice central, BLECharacteristic characteristic) {
  wifiType = wifiTypeCharacteristic.value();

  if (isWifiConncInfoFilled()) wifiInit();
}

void wifiSSIDCharacteristicWritten(BLEDevice central, BLECharacteristic characteristic) {
  wifiSSID = wifiSSIDCharacteristic.value();
  
  if (isWifiConncInfoFilled()) wifiInit();
}

void wifiUserCharacteristicWritten(BLEDevice central, BLECharacteristic characteristic) {
  wifiUser = wifiUserCharacteristic.value();
}

void wifiPassCharacteristicWritten(BLEDevice central, BLECharacteristic characteristic) {
  wifiPass = wifiPassCharacteristic.value();
  if (isWifiConncInfoFilled()) wifiInit();
}
