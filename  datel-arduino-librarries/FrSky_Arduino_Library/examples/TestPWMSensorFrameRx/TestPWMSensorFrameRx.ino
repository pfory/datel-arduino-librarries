/*
    Tests PWM Sensor Frame Rx.
    
    Receives bytes representing data sent from the FrSky FLVS-01 sensor,
    and tries detect frames from the byte flow.
*/

#include <inttypes.h>
#include "PWMSensorFrameRx.h"

extern void receivedFrame(frsky::PWMSensorFrame* frame);

// Frame receiver
frsky::PWMSensorFrameRx rx = frsky::PWMSensorFrameRx(receivedFrame);

// Test data: FLVS-01
uint8_t flvs01_normal[] =  {
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

// Test data: FLVS-01, stuffed data (and also an invalid CRC)
uint8_t flvs01_stuffed[] =  {
  0x7E, // Start
  0x02, // Total Cell
  0x06, // ID
  0xFE, // Data Low
  0x07, // Data High
  0x00, // Reserved
  0x7D, // CRC Low  (0x7E is changed into 2 bytes: 0x7D, 0x5E)
  0x5E,
  0x7D, // CRC High (0x7D is changed into 2 bytes: 0x7D, 0x3D)
  0x3D, 
  0x7E  // End
};

void printHex(uint8_t value)
{
    if (value < 0x10)
    {
        Serial.print('0');
    }
    Serial.println(value, HEX);
}

void receivedFrame(frsky::PWMSensorFrame* frame)
{
    Serial.println("   Frame received:");
    
    Serial.print("      Cells: ");
    Serial.println(frame->cells, HEX);
    Serial.print("      ID:    0x");
    Serial.println(frame->id, HEX);
    Serial.print("      DATA:  0x");
    Serial.println(frame->data, HEX);
    Serial.print("      R:     0x");
    Serial.println(frame->reserved, HEX);
    Serial.print("      CRC:   0x");
    Serial.println(frame->crc, HEX);
}

void runTest(const char* testName, uint8_t* data, uint8_t length)
{
    // Resets receiving state - not received frames count
    rx.reset();
    
    Serial.println(testName);  
    for (int i = 0; i < length; i++)
    {
        printHex(data[i]);
        
        rx.rxByte(data[i]);
        Serial.print("   State: ");
        Serial.println(rx.getState());
        Serial.print("   Total received frames: ");
        Serial.println(rx.getRxFrameCount());
    }
    Serial.println();
}  

void setup()  
{
    Serial.begin(57600);
  
    runTest("FLVS-01 (normal):",  flvs01_normal,  sizeof(flvs01_normal));
    runTest("FLVS-01 (stuffed):", flvs01_stuffed, sizeof(flvs01_stuffed));
}

void loop()
{
}
