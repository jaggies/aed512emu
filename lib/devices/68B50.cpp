/*
 * m68b50.cpp
 *
 *  Created on: Sep 10, 2018
 *      Author: jmiller
 */

#include <cassert>
#include "68B50.h"

#define CONTROL 0
#define STATUS 0 // reading _control register
#define DATA 1

// These bits control transmit IRQs, that is alert the cpu that the transmit queue
// is empty so it can send the next byte.
#define TX_IRQ_MASK (M68B50::CR6 | M68B50::CR5)
#define TX_IRQ_ENABLED (M68B50::CR5)// CR5 | !CR6

uint8_t
M68B50::read(int offset) {
    assert(offset < size());
    switch (offset) {
        case STATUS:
            return _status;
        case DATA:
            _status &= ~(RDRF | IRQW); // data removed from Rx buffer
            return _rxdata;
        default:
            return 0;
    }
}

void
M68B50::write(int offset, uint8_t value) {
    assert(offset < size());
    switch (offset) {
        case CONTROL:
            _control = value;
            break;
        case DATA:
            _txdata = value;
            _status &= ~TDRE; // data now in buffer
            maybeSendNow(_txdata);
            break;
    }
}

void
M68B50::maybeSendNow(uint8_t data) {
    if (_send && _send(data)) {
        _status |= TDRE; // now it's empty
        if ((_control & TX_IRQ_MASK) == TX_IRQ_ENABLED) {
           _status |= IRQW;
           if (_irq) {
               _irq();
           }
        }
    }
}

// Host to serial port. Returns true if byte can be received.
bool
M68B50::receive(uint8_t byte) {
    // The manual states that RTS# is low when CR6 is low or CR5 and CR6 are both high
    const int mask = (CR6 | CR5);
    bool requestToSend = ((_control & CR6) == 0) || (_control & mask) == mask;

    if (!requestToSend || (_status & RDRF)) {
        return false; // receiver data register full or not ready
    }

    if (_status && requestToSend) {
        _rxdata = byte;
        _status |= RDRF; // something is now in the receive buffer
        if (_control & CR7) {
            _status |= IRQW; // irq enabled and waiting. TODO: set flag even if IRQ is disabled?
            if (_irq) {
                _irq(); // Invoke IRQ
            }
        }
        return true;
    }
    return false;
}

// Serial port to host. Returns true if data was available
bool
M68B50::transmit(uint8_t* byte) {
    if (!(_status & TDRE)) { // transmit data register contains data
        *byte = _txdata;
        _status |= TDRE; // now it's empty
        if ((_control & TX_IRQ_MASK) == TX_IRQ_ENABLED) {
            _status |= IRQW;
        }
        return true;
    }
    return false;
}
