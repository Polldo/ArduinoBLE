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

#include "BLERemoteAdvertisingData.h"

BLERemoteAdvertisingData::BLERemoteAdvertisingData() :
  _rawData()
{
}

BLERemoteAdvertisingData::~BLERemoteAdvertisingData()
{
}

const BLEAdvertisingRawData BLERemoteAdvertisingData::rawData() const
{
  return _rawData;
}

void BLERemoteAdvertisingData::setData(int length, const uint8_t *data)
{
  // Saturate data length
  if (length > MAX_AD_DATA_LENGTH) {
    length = MAX_AD_DATA_LENGTH;
  }
  _rawData.length = length;
  memcpy(_rawData.data, data, length);
}

bool BLERemoteAdvertisingData::hasAdvertisedServiceUuid() const
{
  return hasAdvertisedServiceUuid(0);
}

bool BLERemoteAdvertisingData::hasAdvertisedServiceUuid(int index) const
{
  return (advertisedServiceUuid(index).length() > 0);
}

bool BLERemoteAdvertisingData::hasManufacturerData() const
{
  return (manufacturerData().length > 0);
}

int BLERemoteAdvertisingData::advertisedServiceUuidCount() const
{
  int dataLength = _rawData.length;
  const uint8_t* data = _rawData.data;
  int advertisedServiceCount = 0;

  for (unsigned char i = 0; i < dataLength;) {
    int eirLength = data[i++];
    int eirType = data[i++];

    if (eirType == BLEFieldIncompleteAdvertisedService16 || eirType == BLEFieldCompleteAdvertisedService16 || 
        eirType == BLEFieldIncompleteAdvertisedService128 || eirType == BLEFieldCompleteAdvertisedService128) {

      int uuidLength;

      if (eirType == BLEFieldIncompleteAdvertisedService16 || eirType == BLEFieldCompleteAdvertisedService16) {
        uuidLength = 2;
      } else /*if (eirType == BLEFieldIncompleteAdvertisedService128 || eirType == BLEFieldCompleteAdvertisedService128)*/ {
        uuidLength = 16;
      }

      for (int j = 0; j < (eirLength - 1); j += uuidLength) {
        advertisedServiceCount++;
      }
    }

    i += (eirLength - 1);
  }

  return advertisedServiceCount;
}

String BLERemoteAdvertisingData::advertisedServiceUuid(int index) const
{
  int dataLength = _rawData.length;
  const uint8_t* data = _rawData.data;
  String serviceUuid;
  int uuidIndex = 0;

  for (unsigned char i = 0; i < dataLength;) {
    int eirLength = data[i++];
    int eirType = data[i++];

    if (eirType == BLEFieldIncompleteAdvertisedService16 || eirType == BLEFieldCompleteAdvertisedService16 || 
        eirType == BLEFieldIncompleteAdvertisedService128 || eirType == BLEFieldCompleteAdvertisedService128) {

      int uuidLength;

      if (eirType == BLEFieldIncompleteAdvertisedService16 || eirType == BLEFieldCompleteAdvertisedService16) {
        uuidLength = 2;
      } else /*if (eirType == BLEFieldIncompleteAdvertisedService128 || eirType == BLEFieldCompleteAdvertisedService128)*/ {
        uuidLength = 16;
      }

      for (int j = 0; j < (eirLength - 1); j += uuidLength) {
        if (uuidIndex == index) {
          serviceUuid = BLEUuid::uuidToString(&data[i + j * uuidLength], uuidLength);
        }

        uuidIndex++;
      }
    }

    i += (eirLength - 1);
  }

  return serviceUuid;
}

String BLERemoteAdvertisingData::localName() const
{
  int dataLength = _rawData.length;
  const uint8_t* data = _rawData.data;
  String localName = "";

  for (int i = 0; i < dataLength;) {
    int eirLength = data[i++];
    int eirType = data[i++];

    if (eirType == BLEFieldCompleteLocalName || eirType == BLEFieldShortLocalName) {
      localName.reserve(eirLength - 1);

      for (int j = 0; j < (eirLength - 1); j++) {
        localName += (char)data[i + j];
      }
      break;
    }

    i += (eirLength - 1);
  }

  return localName;
}

BLEAdvertisingRawData BLERemoteAdvertisingData::manufacturerData() const
{
  int dataLength = _rawData.length;
  const uint8_t* data = _rawData.data;
  BLEAdvertisingRawData manufacturerData;
  manufacturerData.length = 0;

  for (int i = 0; i < dataLength;) {
    int eirLength = data[i++];
    int eirType = data[i++];

    if (eirType == BLEFieldManufacturerData) {
      manufacturerData.length = eirLength - 1;

      memcpy(manufacturerData.data, &data[i], manufacturerData.length);
      break;
    }

    i += (eirLength - 1);
  }

  return manufacturerData;
}
