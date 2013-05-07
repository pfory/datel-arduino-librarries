#ifndef FRSKY_CRC_H
#define	FRSKY_CRC_H

namespace frsky
{
    
//! See  "Frame Protocol of FrSky Lipo Voltage Sensor and Ampere Sensor" (http://www.frsky-rc.com)
extern uint16_t FcsCheck(uint8_t *pdata,short num);

}

#endif	// FRSKY_CRC_H
