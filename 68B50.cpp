/*
 * m68b50.cpp
 *
 *  Created on: Sep 10, 2018
 *      Author: jmiller
 */

#include "68B50.h"

uint8_t
M68B50::read(int offset) {
    uint8_t result = _store[offset];
    return result;
}

void
M68B50::write(int offset, uint8_t value) {
    _store[offset] = value;
}
