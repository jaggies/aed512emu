/*
 * m68b50.cpp
 *
 *  Created on: Sep 10, 2018
 *      Author: jmiller
 */

#include <cassert>
#include "68B50.h"

#define CONTROL 0
#define STATUS 0 // reading control register
#define DATA 1

uint8_t
M68B50::read(int offset) {
    assert(offset < size());
    switch (offset) {
        case STATUS:
            return status;
        case DATA:
            status &= ~(RDRF | IRQ); // data removed from Rx buffer
            return rxdata;
        default:
            return 0;
    }
}

void
M68B50::write(int offset, uint8_t value) {
    assert(offset < size());
    switch (offset) {
        case CONTROL:
            control = value;
            break;
        case DATA:
            txdata = value;
            status &= ~TDRE; // data now in buffer
            break;
    }
}

// Input to serial port. Returns true if byte can be received.
bool
M68B50::receive(uint8_t byte) {
    if (status & RDRF) {
        return false; // receiver data register full
    }
    if (status & CTS) {
        rxdata = byte;
        status |= RDRF; // something is now in the receive buffer
        if (control & CR7) {
            status |= IRQ; // IRQ enabled
        }
        return true;
    }
    return false;
}

// Output from serial port. Returns true if data was available
bool
M68B50::transmit(uint8_t* byte) {
    const uint8_t IRQ_ENABLED = (CR5); // CR5 | !CR6
    const uint8_t IRQ_MASK = (CR6 | CR5);
    if (!(status & TDRE)) { // transmit data register contains data
        *byte = txdata;
        status |= TDRE; // now it's empty
        if ((control & IRQ_MASK) == IRQ_ENABLED) {
            status |= IRQ;
        }
        return true;
    }
    return false;
}
