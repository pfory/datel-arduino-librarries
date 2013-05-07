/*
    Tests PWM Sensor Pin Listener.

    Detects waves sent from the FrSky FLVS-01 and FAS-01 (not tested),
    and tries to interpret the corresponding byte value.

    Connect according to the FrSky documentation for the sensor hub.
    This sketch assumes that the input is connected to Arduino Digital Pin 2.
*/

#include <inttypes.h>
#include "PWMSensorPinRx.h"

// Wave length base for the sensor byte protocol
#define WAVELENGTH_BASE_MICROS 5000

// Offset to the received wave length required to get it to work for me
// (Since I don't know how harware works?)
#define OFFSET_MICROS 848

// The pin listener detects waves and calls back to waveReceived upon receiving
frsky::PWMSensorPinListener pinListener = frsky::PWMSensorPinListener(waveReceived);

void waveReceived(uint16_t waveLengthMicros)
{
    // Fix due to timer problems
     unsigned int lastWaveLengthMicros = waveLengthMicros + OFFSET_MICROS;
     
    if (lastWaveLengthMicros < WAVELENGTH_BASE_MICROS)
    {
        Serial.println();
        Serial.print('-');
    }
    else if (lastWaveLengthMicros > (255 * 8) + WAVELENGTH_BASE_MICROS + 4 )
    {
        Serial.print('+');
    }
    else
    {
        // The value of hexadecimal code for each byte multiplied by 8 (error tolerance of 4us) and
        // plus 5000, with unit of us, is transformed into rectangular wave of corresponding period.
        long value = (lastWaveLengthMicros - WAVELENGTH_BASE_MICROS) / 8;
        if (value < 0x10)
        {
            Serial.print("0");
        }
        String hexValue = String(value, HEX);
        Serial.print(hexValue);
    }
    Serial.print(" (");
    Serial.print(lastWaveLengthMicros);
    Serial.println(")");    
}

void setup()  
{
  Serial.begin(57600);
  Serial.println("Serial initialized.");
  
  // Setup D2 as input
  // Setup Timer2 as a counter for the wave lengths
  pinListener.init();
  Serial.println("Pin listener initialized.");
}

void loop()
{
    // Calls back to the waveReceived function when a wave is received
    pinListener.poll();
}

