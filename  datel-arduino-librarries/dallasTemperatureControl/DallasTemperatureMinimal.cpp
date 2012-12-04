// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.

// Version 3.7.2 modified on Dec 6, 2011 to support Arduino 1.0
// See Includes...
// Modified by Jordan Hochenbaum

#include "DallasTemperatureMinimal.h"

#include "Arduino.h"   

DallasTemperatureMinimal::DallasTemperatureMinimal(OneWire* _oneWire)
  #if REQUIRESALARMS
  : _AlarmHandler(&defaultAlarmHandler)
  #endif
{
  _wire = _oneWire;
  devices = 0;
}

// initialise the bus
void DallasTemperatureMinimal::begin(void)
{
  DeviceAddress deviceAddress;

  _wire->reset_search();
  devices = 0; // Reset the number of devices when we enumerate wire devices

  while (_wire->search(deviceAddress))
  {
    if (validAddress(deviceAddress))
    {
      ScratchPad scratchPad;

      readScratchPad(deviceAddress, scratchPad);

      devices++;
    }
  }
}

// returns the number of devices found on the bus
uint8_t DallasTemperatureMinimal::getDeviceCount(void)
{
  return devices;
}

// returns true if address is valid
bool DallasTemperatureMinimal::validAddress(uint8_t* deviceAddress)
{
  return (_wire->crc8(deviceAddress, 7) == deviceAddress[7]);
}

// finds an address at a given index on the bus
// returns true if the device was found
bool DallasTemperatureMinimal::getAddress(uint8_t* deviceAddress, uint8_t index)
{
  uint8_t depth = 0;

  _wire->reset_search();

  while (depth <= index && _wire->search(deviceAddress))
  {
    if (depth == index && validAddress(deviceAddress)) return true;
    depth++;
  }

  return false;
}

// attempt to determine if the device at the given address is connected to the bus
bool DallasTemperatureMinimal::isConnected(uint8_t* deviceAddress)
{
  ScratchPad scratchPad;
  return isConnected(deviceAddress, scratchPad);
}

// attempt to determine if the device at the given address is connected to the bus
// also allows for updating the read scratchpad
bool DallasTemperatureMinimal::isConnected(uint8_t* deviceAddress, uint8_t* scratchPad)
{
  readScratchPad(deviceAddress, scratchPad);
  return (_wire->crc8(scratchPad, 8) == scratchPad[SCRATCHPAD_CRC]);
}

// read device's scratch pad
void DallasTemperatureMinimal::readScratchPad(uint8_t* deviceAddress, uint8_t* scratchPad)
{
  // send the command
  _wire->reset();
  _wire->select(deviceAddress);
  _wire->write(READSCRATCH);

  // TODO => collect all comments &  use simple loop
  // byte 0: temperature LSB  
  // byte 1: temperature MSB
  // byte 2: high alarm temp
  // byte 3: low alarm temp
  // byte 4: DS18S20: store for crc
  //         DS18B20 & DS1822: configuration register
  // byte 5: internal use & crc
  // byte 6: DS18S20: COUNT_REMAIN
  //         DS18B20 & DS1822: store for crc
  // byte 7: DS18S20: COUNT_PER_C
  //         DS18B20 & DS1822: store for crc
  // byte 8: SCRATCHPAD_CRC
  //
  // for(int i=0; i<9; i++)
  // {
  //   scratchPad[i] = _wire->read();
  // }

  
  // read the response

  // byte 0: temperature LSB
  scratchPad[TEMP_LSB] = _wire->read();

  // byte 1: temperature MSB
  scratchPad[TEMP_MSB] = _wire->read();

  // byte 2: high alarm temp
  scratchPad[HIGH_ALARM_TEMP] = _wire->read();

  // byte 3: low alarm temp
  scratchPad[LOW_ALARM_TEMP] = _wire->read();

  // byte 4:
  // DS18S20: store for crc
  // DS18B20 & DS1822: configuration register
  scratchPad[CONFIGURATION] = _wire->read();

  // byte 5:
  // internal use & crc
  scratchPad[INTERNAL_BYTE] = _wire->read();

  // byte 6:
  // DS18S20: COUNT_REMAIN
  // DS18B20 & DS1822: store for crc
  scratchPad[COUNT_REMAIN] = _wire->read();

  // byte 7:
  // DS18S20: COUNT_PER_C
  // DS18B20 & DS1822: store for crc
  scratchPad[COUNT_PER_C] = _wire->read();

  // byte 8:
  // SCTRACHPAD_CRC
  scratchPad[SCRATCHPAD_CRC] = _wire->read();

  _wire->reset();
}

// writes device's scratch pad
void DallasTemperatureMinimal::writeScratchPad(uint8_t* deviceAddress, const uint8_t* scratchPad)
{
  _wire->reset();
  _wire->select(deviceAddress);
  _wire->write(WRITESCRATCH);
  _wire->write(scratchPad[HIGH_ALARM_TEMP]); // high alarm temp
  _wire->write(scratchPad[LOW_ALARM_TEMP]); // low alarm temp
  // DS18S20 does not use the configuration register
  if (deviceAddress[0] != DS18S20MODEL) _wire->write(scratchPad[CONFIGURATION]); // configuration
  _wire->reset();
  // save the newly written values to eeprom
  _wire->write(COPYSCRATCH, false);
  _wire->reset();
}


// sends command for all devices on the bus to perform a temperature conversion
void DallasTemperatureMinimal::requestTemperatures()
{
  _wire->reset();
  _wire->skip();
  _wire->write(STARTCONVO, false);

  return;
}

// reads scratchpad and returns the temperature in degrees C
float DallasTemperatureMinimal::calculateTemperature(uint8_t* deviceAddress, uint8_t* scratchPad)
{
  int16_t rawTemperature = (((int16_t)scratchPad[TEMP_MSB]) << 8) | scratchPad[TEMP_LSB];

  switch (deviceAddress[0])
  {
    case DS18B20MODEL:
    case DS1822MODEL:
      switch (scratchPad[CONFIGURATION])
      {
        case TEMP_12_BIT:
          return (float)rawTemperature * 0.0625;
          break;
        case TEMP_11_BIT:
          return (float)(rawTemperature >> 1) * 0.125;
          break;
        case TEMP_10_BIT:
          return (float)(rawTemperature >> 2) * 0.25;
          break;
        case TEMP_9_BIT:
          return (float)(rawTemperature >> 3) * 0.5;
          break;
      }
      break;
    case DS18S20MODEL:
      /*

      Resolutions greater than 9 bits can be calculated using the data from
      the temperature, COUNT REMAIN and COUNT PER �C registers in the
      scratchpad. Note that the COUNT PER �C register is hard-wired to 16
      (10h). After reading the scratchpad, the TEMP_READ value is obtained
      by truncating the 0.5�C bit (bit 0) from the temperature data. The
      extended resolution temperature can then be calculated using the
      following equation:

                                       COUNT_PER_C - COUNT_REMAIN
      TEMPERATURE = TEMP_READ - 0.25 + --------------------------
                                               COUNT_PER_C
      */

      // Good spot. Thanks Nic Johns for your contribution
      return (float)(rawTemperature >> 1) - 0.25 +((float)(scratchPad[COUNT_PER_C] - scratchPad[COUNT_REMAIN]) / (float)scratchPad[COUNT_PER_C] );
      break;
  }
}

// returns temperature in degrees C or DEVICE_DISCONNECTED if the
// device's scratch pad cannot be read successfully.
// the numeric value of DEVICE_DISCONNECTED is defined in
// DallasTemperatureMinimal.h. It is a large negative number outside the
// operating range of the device
float DallasTemperatureMinimal::getTempC(uint8_t* deviceAddress)
{
  // TODO: Multiple devices (up to 64) on the same bus may take 
  //       some time to negotiate a response
  // What happens in case of collision?

  ScratchPad scratchPad;
  if (isConnected(deviceAddress, scratchPad)) return calculateTemperature(deviceAddress, scratchPad);
  return DEVICE_DISCONNECTED;
}