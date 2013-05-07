/*
    Tests FrSKY CRC.
*/

#include <inttypes.h>
#include "FrSkyCRC.h"

uint8_t data[] =  { 
  0x7E, // Start
  0x02, // Total Cell
  0x06, // ID
  0xFE, // Data Low
  0x07, // Data High
  0x00, // Reserved
  
  0x96, // CRC Low
  0x70, // CRC High
  0x7E  // End
};

// CRCL = 0x96
// CRCH = 0x70
const uint16_t expectedCRC = 0x7096; //data[6] | (data[7] << 8);
const uint8_t dataLength = 6;

void printHex(uint8_t value)
{
    if (value < 0x10)
    {
        Serial.print('0');
    }
    Serial.println(value, HEX);
}

void setup()  
{
  Serial.begin(57600);
  
  Serial.println("Testing CRC:");
  for (int i = 0; i < sizeof(data); i++)
  {
      printHex(data[i]);
  }
  
  Serial.println("=>");
 
  uint16_t crc = frsky::FcsCheck(data, dataLength);
  
  Serial.print("Calculated: ");
  Serial.println(crc, HEX);
  Serial.print("Expected:   ");
  Serial.println(expectedCRC, HEX);
  
  Serial.println("Finished.");  
}

void loop()
{
}
