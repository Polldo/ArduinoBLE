#include <ArduinoBLE.h>

struct __attribute__ ((packed)) TPMSData {
  uint16_t manufacturer;  // This may be reversed
  uint8_t sensorNumber;
  uint16_t addressPrefix;
  uint16_t sensorAddressL;
  uint8_t sensorAddressH;
  uint32_t pressure;
  uint32_t temperature;
  uint8_t battery;
  uint8_t alarm;
};

void setup() {
  Serial.begin(9600);
  while (!Serial);

  // begin initialization
  if (!BLE.begin()) {
    Serial.println("starting BLE failed!");

    while (1);
  }

  Serial.println("BLE Central scan");

  // start scanning for peripheral
  BLE.scan();
}

void loop() {
  // check if a peripheral has been discovered
  BLEDevice peripheral = BLE.available();

  if (peripheral) {
    // discovered a peripheral
    Serial.println("Discovered a peripheral");
    Serial.println("-----------------------");

    // print address
    Serial.print("Address: ");
    Serial.println(peripheral.address());

    // print the local name, if present
    if (peripheral.hasLocalName()) {
      Serial.print("Local Name: ");
      Serial.println(peripheral.localName());
    }

    if (peripheral.hasManufacturerData()) {
      TPMSData data;
      Serial.print("TPMS manufacturer: ");
      Serial.println(data.manufacturer, HEX);
      peripheral.manufacturerData(sizeof(data), (uint8_t*)&data);
      Serial.print("TPMS pressure: ");
      Serial.println(data.pressure, HEX);
    }


    Serial.println();
  }
}
