/*
  This file is part of the ArduinoBLE library.
  Copyright (c) 2018 Arduino SA. All rights reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "utility/ATT.h"
#include "utility/BLEUuid.h"
#include "utility/HCI.h"

#include "remote/BLERemoteDevice.h"

#include "BLEDevice.h"

BLEDevice::BLEDevice() :
  _rssi(127),
  _discovered(false)
{
  memset(_address, 0x00, sizeof(_address));
}

BLEDevice::BLEDevice(uint8_t addressType, uint8_t address[6]) :
  _addressType(addressType),
  _rssi(127),
  _discovered(false)
{
  memcpy(_address, address, sizeof(_address));
}

BLEDevice::~BLEDevice()
{
}

void BLEDevice::poll()
{
  HCI.poll();
}

void BLEDevice::poll(unsigned long timeout)
{
  HCI.poll(timeout);
}

bool BLEDevice::connected() const
{
  HCI.poll();

  if (!(*this)) {
    return false;
  }

  return ATT.connected(_addressType, _address);
}

bool BLEDevice::disconnect()
{
  return ATT.disconnect(_addressType, _address);
}

String BLEDevice::address() const
{
  char result[18];
  sprintf(result, "%02x:%02x:%02x:%02x:%02x:%02x", _address[5], _address[4], _address[3], _address[2], _address[1], _address[0]);

  return result;
}

bool BLEDevice::hasLocalName() const
{
  return (localName().length() > 0);
}

bool BLEDevice::hasAdvertisedServiceUuid() const
{
  return hasAdvertisedServiceUuid(0);
}

bool BLEDevice::hasAdvertisedServiceUuid(int index) const
{
  return (advertisedServiceUuid(index).length() > 0);
}

int BLEDevice::advertisedServiceUuidCount() const
{
  int count = _advertisingData.advertisedServiceUuidCount();
  count += _scanResponseData.advertisedServiceUuidCount();
  return count;
}

String BLEDevice::localName() const
{
  String localName = _advertisingData.localName();
  if (!localName.length()) {
    localName = _scanResponseData.localName();
  }
  return localName;
}

String BLEDevice::advertisedServiceUuid() const
{
  return advertisedServiceUuid(0);
}

String BLEDevice::advertisedServiceUuid(int index) const
{
  // Analyze advertising data count to understand where the uuid should be (advData or scanData)
  String serviceUuid = "";
  int countServicesInAdvertising = _advertisingData.advertisedServiceUuidCount();
  if (index < countServicesInAdvertising) {
    serviceUuid = _advertisingData.advertisedServiceUuid(index);
  } else {
    index -= countServicesInAdvertising;
    serviceUuid = _scanResponseData.advertisedServiceUuid(index);
  }
  return serviceUuid;
}

const BLEAdvertisingRawData BLEDevice::advertisingData() const
{
  return _advertisingData.rawData();
}

const BLEAdvertisingRawData BLEDevice::scanResponseData() const
{
  return _scanResponseData.rawData();
}

int BLEDevice::rssi()
{
  uint16_t handle = ATT.connectionHandle(_addressType, _address);

  if (handle != 0xffff) {
    return HCI.readRssi(handle);
  }

  return _rssi;
}

bool BLEDevice::connect()
{
  return ATT.connect(_addressType, _address);
}

bool BLEDevice::discoverAttributes()
{
  return ATT.discoverAttributes(_addressType, _address, NULL);
}

bool BLEDevice::discoverService(const char* serviceUuid)
{
  return ATT.discoverAttributes(_addressType, _address, serviceUuid);
}

BLEDevice::operator bool() const
{
  uint8_t zeros[6] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,};

  return (memcmp(_address, zeros, sizeof(zeros)) != 0);
}

bool BLEDevice::operator==(const BLEDevice& rhs) const
{
  return ((_addressType == rhs._addressType) && memcmp(_address, rhs._address, sizeof(_address)) == 0);
}

bool BLEDevice::operator!=(const BLEDevice& rhs) const
{
  return ((_addressType != rhs._addressType) || memcmp(_address, rhs._address, sizeof(_address)) != 0);
}

String BLEDevice::deviceName()
{
  BLERemoteDevice* device = ATT.device(_addressType, _address);

  if (device) {
    BLEService genericAccessService = service("1800");

    if (genericAccessService) {
      BLECharacteristic deviceNameCharacteristic = genericAccessService.characteristic("2a00");

      if (deviceNameCharacteristic) {
        deviceNameCharacteristic.read();

        String result;
        int valueLength = deviceNameCharacteristic.valueLength();
        const char* value = (const char*)deviceNameCharacteristic.value();

        result.reserve(valueLength);

        for (int i = 0; i < valueLength; i++) {
          result += value[i];
        }

        return result;
      }
    }
  }

  return "";
}

int BLEDevice::appearance()
{
  BLERemoteDevice* device = ATT.device(_addressType, _address);

  if (device) {
    BLEService genericAccessService = service("1801");

    if (genericAccessService) {
      BLECharacteristic appearanceCharacteristic = genericAccessService.characteristic("2a01");

      if (appearanceCharacteristic) {
        appearanceCharacteristic.read();

        uint16_t result = 0;

        memcpy  (&result, appearanceCharacteristic.value(), min((int)sizeof(result), appearanceCharacteristic.valueLength()));

        return result;
      }
    }
  }

  return 0;
}

int BLEDevice::serviceCount() const
{
  BLERemoteDevice* device = ATT.device(_addressType, _address);

  if (device) {
    return device->serviceCount();
  }

  return 0;
}

bool BLEDevice::hasService(const char* uuid) const
{
  return hasService(uuid, 0);
}

bool BLEDevice::hasService(const char* uuid, int index) const
{
  BLERemoteDevice* device = ATT.device(_addressType, _address);

  if (device) {
    int count = 0;
    int numServices = device->serviceCount();

    for (int i = 0; i < numServices; i++) {
      BLERemoteService* s = device->service(i);

      if (strcasecmp(uuid, s->uuid()) == 0) {
        if (count == index) {
          return true;
        }

        count++;
      }
    }
  }

  return false;
}

BLEService BLEDevice::service(int index) const
{
  BLERemoteDevice* device = ATT.device(_addressType, _address);

  if (device) {
    if (index < (int)device->serviceCount()) {
      return BLEService(device->service(index));
    }
  }

  return BLEService();
}

BLEService BLEDevice::service(const char * uuid) const
{
  return service(uuid, 0);
}

BLEService BLEDevice::service(const char * uuid, int index) const
{
  BLERemoteDevice* device = ATT.device(_addressType, _address);

  if (device) {
    int count = 0;
    int numServices = device->serviceCount();

    for (int i = 0; i < numServices; i++) {
      BLERemoteService* s = device->service(i);

      if (strcasecmp(uuid, s->uuid()) == 0) {
        if (count == index) {
          return BLEService(s);
        }

        count++;
      }
    }
  }

  return BLEService();
}

int BLEDevice::characteristicCount() const
{
  BLERemoteDevice* device = ATT.device(_addressType, _address);

  if (device) {
    int result = 0;
    int numServices = device->serviceCount();

    for (int i = 0; i < numServices; i++) {
      result += device->service(i)->characteristicCount();
    }

    return result;
  }

  return 0;
}

bool BLEDevice::hasCharacteristic(const char* uuid) const
{
  return hasCharacteristic(uuid, 0);
}

bool BLEDevice::hasCharacteristic(const char* uuid, int index) const
{
  BLERemoteDevice* device = ATT.device(_addressType, _address);

  if (device) {
    int count = 0;
    int numServices = device->serviceCount();

    for (int i = 0; i < numServices; i++) {
      BLERemoteService* s = device->service(i);

      int numCharacteristics = s->characteristicCount();

      for (int j = 0; j < numCharacteristics; j++) {
        BLERemoteCharacteristic* c = s->characteristic(j);


        if (strcasecmp(c->uuid(), uuid) == 0) {
          if (count == index) {
            return true;
          }
        }

        count++;
      }
    }
  }

  return false;
}

BLECharacteristic BLEDevice::characteristic(int index) const
{
  BLERemoteDevice* device = ATT.device(_addressType, _address);

  if (device) {
    int count = 0;
    int numServices = device->serviceCount();

    for (int i = 0; i < numServices; i++) {
      BLERemoteService* s = device->service(i);

      int numCharacteristics = s->characteristicCount();

      for (int j = 0; j < numCharacteristics; j++) {
        if (count == index) {
          BLERemoteCharacteristic* c = s->characteristic(j);

          return BLECharacteristic(c);
        }

        count++;
      }
    }
  }

  return BLECharacteristic();
}

BLECharacteristic BLEDevice::characteristic(const char * uuid) const
{
  return characteristic(uuid, 0);
}

BLECharacteristic BLEDevice::characteristic(const char * uuid, int index) const
{
  BLERemoteDevice* device = ATT.device(_addressType, _address);

  if (device) {
    int count = 0;
    int numServices = device->serviceCount();

    for (int i = 0; i < numServices; i++) {
      BLERemoteService* s = device->service(i);

      int numCharacteristics = s->characteristicCount();

      for (int j = 0; j < numCharacteristics; j++) {
        BLERemoteCharacteristic* c = s->characteristic(j);

        if (strcasecmp(c->uuid(), uuid) == 0) {
          if (count == index) {

            return BLECharacteristic(c);
          }

          count++;
        }
      }
    }
  }

  return BLECharacteristic();
}

bool BLEDevice::hasAddress(uint8_t addressType, uint8_t address[6])
{
  return (_addressType == addressType) && (memcmp(_address, address, sizeof(_address)) == 0);
}

void BLEDevice::setAdvertisingData(uint8_t eirDataLength, uint8_t eirData[], int8_t rssi)
{
  _discovered = true;
  _advertisingData.setData(eirDataLength, eirData);
  _rssi = rssi;
}

void BLEDevice::setScanResponseData(uint8_t eirDataLength, uint8_t eirData[], int8_t rssi)
{
  _discovered = true;
  _scanResponseData.setData(eirDataLength, eirData);
  _rssi = rssi;
}

bool BLEDevice::discovered()
{
  return _discovered;
}

