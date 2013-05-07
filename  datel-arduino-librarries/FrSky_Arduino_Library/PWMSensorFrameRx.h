#ifndef FRSKY_PWMSENSORFRAMERX_H
#define	FRSKY_PWMSENSORFRAMERX_H

#include <inttypes.h>

namespace frsky
{

//! PWMSensorFrame from FrSky FLVS-01 or FAS-01
typedef struct
{
    uint8_t cells;
    uint8_t id;
    uint16_t data;
    uint8_t reserved;
    uint16_t crc;
    bool crcOk;
} PWMSensorFrame;

//! Function pointer for processing received PWMSensor frames
typedef void (*PWMSensorFrameReceivedDelegate)(PWMSensorFrame* frame);

//! A class parsing frames received from FrSky sensors using PWM communication
class PWMSensorFrameRx {
  public:
    typedef enum {
        RXSTATE_AWAITING_START_FLAG = 0,
        RXSTATE_RECEIVING           = 1,
        RXSTATE_RECEIVING_STUFFED   = 2,
        RXSTATE_FRAME_RECEIVED      = 3                
    } RxState;
    
    PWMSensorFrameRx(PWMSensorFrameReceivedDelegate onFrameReceived);
    
    void rxByte(uint8_t value);
    void reset();

    /**
     * @brief Returns the current state for the receiver.
     */
    RxState getState();
    
    uint16_t getRxFrameCount();
    
  private:
    void processFrame();
      
    static const uint8_t flag = 0x7E;
    static const uint8_t stuffed = 0x7D;
    
    PWMSensorFrameReceivedDelegate frameRxDelegate;    
    uint16_t rxFrameCount;
    uint8_t rxBuffer[9];
    uint8_t rxBufferIndex;
    RxState state;
    bool isStuffed;
};

}
#endif	// FRSKY_PWMSENSORFRAMERX_H
