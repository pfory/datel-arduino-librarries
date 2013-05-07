#include <Arduino.h>
#include "PWMSensorPinRx.h"

namespace frsky
{

PWMSensorPinListener::PWMSensorPinListener(PWMSensorPinByteReceivedDelegate onWaveReceived)
{
    waveRxDelegate = onWaveReceived;
}

void PWMSensorPinListener::poll()
{
  unsigned int lastWaveLengthTicks = pinrx_waveLengthTicks;
  if (lastWaveLengthTicks != 0)
  {
     pinrx_waveLengthTicks = 0;
     if (waveRxDelegate != NULL)
     {
         waveRxDelegate(lastWaveLengthTicks / 2);
     }
  }
}

volatile unsigned int pinrx_waveLengthTicks;
volatile boolean hasTimerOverflown;

void pinrx_StateChanged()
{
    if (PIND & (1 << 2))
    {
        // HIGH: Wave start
        // Set the timercCounter to 0
        TCNT1 = 0;
        hasTimerOverflown = false;
    } 
    else if (!hasTimerOverflown)
    {
        // LOW: Wave end
        // The timer counter contains the wave length ticks
        pinrx_waveLengthTicks = TCNT1;
    }
}

ISR(TIMER1_OVF_vect)
{
    hasTimerOverflown = true;
}

void PWMSensorPinListener::init()
{
    // D2: Sensor input pin
    pinMode(PIN2, INPUT);
    
    // enable internal pull-up
    digitalWrite(PIN2, HIGH);
    
    // attach interrupt
    attachInterrupt(0, pinrx_StateChanged, CHANGE);
    
    // Timer 2 counts the pulse wave length
    
    // TIMSK1: Timer/Counter1 Interrupt Mask Register
    //  - TICIE1:  Timer/Counter1 Input Capture Interrupt Enable
    //  - OCIE1A:  Timer/Counter1 Output CompareA Match Interrupt Enable
    //  - OCIE1B:  Timer/Counter1 Output CompareB Match Interrupt Enable
    //  - TOIE1:   Timer/Counter1 Overflow Interrupt Enable
    TIMSK1 &= ~((1 << ICIE1 ) | (1 << OCIE1A) | (1 << OCIE1B) | (1 << TOIE1));  

    // Clear the timer count
    TCNT1 = 0;

    // Set normal counting mode 
    TCCR1A = 0;

    // Set prescaler to 8
    TCCR1B &= ~(_BV(CS12) | _BV(CS11) | _BV(CS10));

    //TCCR1B |= ((0 << CS12) | (0 << CS11) | (0 << CS10)); // stopped
    //TCCR1B |= ((0 << CS12) | (0 << CS11) | (1 << CS10)); // 1
    TCCR1B |= ((0 << CS12) | (1 << CS11) | (0 << CS10)); // 8
    //TCCR1B |= ((0 << CS12) | (1 << CS11) | (1 << CS10)); // 64
    //TCCR1B |= ((1 << CS12) | (0 << CS11) | (0 << CS10)); // 256
    //TCCR1B |= ((1 << CS12) | (0 << CS11) | (1 << CS10)); // 1024

    // Enable overflow interrupt
    TIMSK1 |= (1 << TOIE1);
}

}
