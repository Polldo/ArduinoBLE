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

#ifndef _BLE_REMOTE_ADVERTISING_DATA_H_
#define _BLE_REMOTE_ADVERTISING_DATA_H_

#include "BLEAdvertisingData.h"

class BLERemoteAdvertisingData {
public:
  BLERemoteAdvertisingData(); 
  virtual ~BLERemoteAdvertisingData();

  bool hasLocalName() const;
  bool hasAdvertisedServiceUuid() const;
  bool hasAdvertisedServiceUuid(int index) const;

  int advertisedServiceUuidCount() const;
  String advertisedServiceUuid(int index) const;
  String localName() const;

  const BLEAdvertisingRawData rawData();

protected:
  friend class BLEDevice;
  void setData(int length, const uint8_t *data);

private:
  BLEAdvertisingRawData _rawData;
};

#endif
