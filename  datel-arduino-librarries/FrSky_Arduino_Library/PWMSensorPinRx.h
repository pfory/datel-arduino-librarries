#ifndef FRSKY_PWMSENSORPINRX_H
#define	FRSKY_PWMSENSORPINRX_H

namespace frsky
{
    
extern volatile unsigned int pinrx_waveLengthTicks;

//! Function pointer for processing received PWMSensor frames
typedef void (*PWMSensorPinByteReceivedDelegate)(const uint16_t waveLengthMicros);

class PWMSensorPinListener
{
  public:
    PWMSensorPinListener(PWMSensorPinByteReceivedDelegate onWaveReceived);
    void poll();
    
    static void init();
    
  private:
      PWMSensorPinByteReceivedDelegate waveRxDelegate;
};
    
}
#endif	// FRSKY_PWMSENSORPINRX_H
