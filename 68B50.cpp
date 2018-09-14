/*
 * m68b50.cpp
 *
 *  Created on: Sep 10, 2018
 *      Author: jmiller
 */

#include "68B50.h"

#define CONTROL 0
#define DATA 1

uint8_t
M68B50::read(int offset) {
    if ((offset & 1) == CONTROL) {
        return control;
    } else {
        control &= ~RDRF; // data removed from buffer
        return rxdata;
    }
}

void
M68B50::write(int offset, uint8_t value) {
    switch (offset) {
        case CONTROL:
            control = value;
            break;
        case DATA:
            txdata = value;
            control &= ~TDRE; // data now in buffer
            break;
    }
}

// Input to serial port. Returns true if byte can be received.
bool
M68B50::receive(uint8_t byte) {
    const uint8_t IRQ_ENABLED = CR7;
    if (control & RDRF) {
        return false; // receiver data register full
    }

    rxdata = byte;
    control |= RDRF;
    if (control & IRQ_ENABLED) {
        control |= IRQ; // IRQ enabled
    }
    return true;
}

// Output from serial port. Returns true if data was available
bool
M68B50::transmit(uint8_t& byte) {
    const uint8_t IRQ_ENABLED = (CR5); // CR5 | !CR6
    const uint8_t IRQ_MASK = (CR6 | CR5);
    if ((control & TDRE) == 0) { // transmit data register contains data
        byte = txdata;
        control |= TDRE; // now it's empty
        if ((control & IRQ_MASK) == IRQ_ENABLED) {
            control |= IRQ;
        }
        return true;
    }
    return false;
}
