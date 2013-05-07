#include <inttypes.h>
#include <stdio.h>
#include "PWMSensorFrameRx.h"
#include "FrSkyCRC.h"

namespace frsky {

PWMSensorFrameRx::PWMSensorFrameRx(PWMSensorFrameReceivedDelegate onFrameReceived)
{
    this->frameRxDelegate = onFrameReceived;
}

void PWMSensorFrameRx::rxByte(uint8_t value)
{
    if (value == flag)
    {   
        if (state == RXSTATE_AWAITING_START_FLAG)
        {    
            this->rxBuffer[0] = value;
            this->rxBufferIndex = 1;
            this->state = RXSTATE_RECEIVING;
            
            // OK: Start flag
            return;
        }
        else if (state == RXSTATE_RECEIVING &&
                 this->rxBufferIndex == sizeof(rxBuffer) - 1)
        {
            // OK: End flag
            rxFrameCount++;
            this->processFrame();
            this->reset();
            return;
        }
    }
    else if (value == stuffed)
    {
        if (state == RXSTATE_RECEIVING)
        {
            this->state = RXSTATE_RECEIVING_STUFFED;
            // OK: Stuffing indicator
            //     When byte 0x7D is received, discard this byte [...]
            return;
        }        
    }
    else
    {
        if (rxBufferIndex < sizeof(rxBuffer) - 1)
        {        
            if (state == RXSTATE_RECEIVING)
            {
                // OK: Received byte
                this->rxBuffer[rxBufferIndex] = value;
                this->rxBufferIndex++;
                return;
            }
            else if (state == RXSTATE_RECEIVING_STUFFED)
            {
                // OK: Received stuffed byte
                //     [...] and the next byte is XORed with 0x20
                this->rxBuffer[rxBufferIndex] = value ^ 0x20;
                this->rxBufferIndex++;
                this->state = RXSTATE_RECEIVING;
                return;
            }
        }        
    }
    
    // Drop frame
    reset();
}

void PWMSensorFrameRx::reset()
{
    rxBufferIndex = 0;
    state = RXSTATE_AWAITING_START_FLAG;
}

PWMSensorFrameRx::RxState PWMSensorFrameRx::getState()
{
    return state;
}

uint16_t PWMSensorFrameRx::getRxFrameCount()
{
    return rxFrameCount;
}

/* private */
void PWMSensorFrameRx::processFrame()
{
    if (frameRxDelegate != NULL)
    {
        uint16_t calculatedCrc = frsky::FcsCheck(rxBuffer, 6);
    
        PWMSensorFrame frame;
        frame.cells = rxBuffer[1];
        frame.id = rxBuffer[2];
        frame.data = (rxBuffer[4] << 8) | rxBuffer[3];
        frame.reserved = rxBuffer[5];
        frame.crc = (rxBuffer[7] << 8) | rxBuffer[6];

        frame.crcOk = (calculatedCrc == frame.crc);
        
        frameRxDelegate(&frame);
    }    
}

}
