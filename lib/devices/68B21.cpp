/*
 * 68B21.cpp
 *
 *  Created on: Sep 9, 2018
 *      Author: jmiller
 */

#include <iostream>
#include <cassert>
#include "68B21.h"

static bool debug = false;

uint8_t M68B21::read(int offset) {
    assert(offset < 4);
    uint8_t result = 0;
    switch (offset) {
        case 0: // PRA or DDRA
            if (CRA & CRA2) {
                result = (inA & ~DDRA) | (PRA & DDRA);
                CRA &= 0x3f; // read of peripheral register resets IRQ bits
            } else {
                result = DDRA;
            }
            break;
        case 1: // CRA
            result = CRA;
            if (debug) std::cerr << name() << ":CRA read -> " << (int) result << std::endl;
            break;
        case 2: // PRB or DDRB
            if (CRB & CRB2) {
                result = (inB & ~DDRB) | (PRB & DDRB);
                CRB &= 0x3f; // read of peripheral register resets IRQ bits
            } else {
                result = DDRB;
            }
            break;
        case 3: // CRB
            result = CRB;
            if (debug) std::cerr << name() << ":CRB read -> " << (int) result << std::endl;
            break;
    }
    return result;
}

void M68B21::write(int offset, uint8_t value) {
    assert(offset < 4);
    switch (offset) {
        case 0: // PRA or DDRA
            if (CRA & CRA2) {
                PRA = value;
            } else {
                DDRA = value;
            }
        break;
        case 1: { // CRA
            uint8_t changed = (CRA ^ value);
            if (debug && (changed & CRA5)) {
                std::cerr << name() << ":CA2 is " << ((value & CRA5) ? "output" : "input") << std::endl;
            }
            if (debug && (changed & CRA4)) { // output mode
                std::cerr << name() << ":CA2 = " << ((value & CRA3) ? "1" : "0") << std::endl;
            }
            CRA = (CRA & 0xc0) | (value & 0x3f); // two upper bits aren't writeable
        }
        break;
        case 2: // PRB or DDRB
            if (CRB & CRB2) {
                PRB = value;
            } else {
                DDRB = value;
            }
        break;
        case 3: { // CRB
            uint8_t changed = (CRB ^ value);
            if (debug && (changed & CRB5)) {
                std::cerr << name() << ":CB2 is " << ((value & CRB5) ? "output" : "input") << std::endl;
            }
            if (debug && (changed & CRB4)) { // output mode
                std::cerr << name() << ":CB2 = " << ((value & CRB3) ? "1" : "0") << std::endl;
            }
            CRB = (CRB & 0xc0) | (value & 0x3f); // two upper bits aren't writeable
        }
        break;
    }
}
