/*
 * 68B21.cpp
 *
 *  Created on: Sep 9, 2018
 *      Author: jmiller
 */

#include <iostream>
#include <cassert>
#include "68B21.h"

const int CRA2 = (1 << 2);
const int CRB2 = (1 << 2);

uint8_t M68B21::read(int offset) {
    assert(offset < 4);
    uint8_t result = 0;
    switch (offset) {
        case 0: // PRA or DDRA
            if (CRA & CRA2) {
                result = PRA;
                CRA &= 0x3f; // read of peripheral register resets IRQ bits
            } else {
                result = DDRA;
            }
            break;
        case 1: // CRA
            result = CRA;
            break;
        case 2: // PRB or DDRB
            if (CRB & CRB2) {
                result = PRB;
                CRB &= 0x3f; // read of peripheral register resets IRQ bits
            } else {
                result = DDRB;
            }
            break;
        case 3: // CRB
            result = CRB;
            //std::cerr << name() << "[CRB] read -> " << (int) result << std::endl;
            break;
    }
    return result;
}

void M68B21::write(int offset, uint8_t value) {
    assert(offset < 4);
    uint8_t result = 0;
    switch (offset) {
        case 0: // PRA or DDRA
            if (CRA & CRA2) {
                PRA = value;
            } else {
                DDRA = value;
            }
        break;
        case 1: // CRA
            CRA = (CRA & 0xc0) | (value & 0x3f); // two upper bits aren't writeable
        break;
        case 2: // PRB or DDRB
            if (CRB & CRB2) {
                PRB = value;
            } else {
                DDRB = value;
            }
        break;
        case 3: // CRB
            CRB = (CRB & 0xc0) | (value & 0x3f); // two upper bits aren't writeable
        break;
    }
}
