/*
 * 68B21.cpp
 *
 *  Created on: Sep 9, 2018
 *      Author: jmiller
 */

#include <iostream>
#include "68B21.h"

uint8_t
M68B21::read(int offset) {
    uint8_t result = _store[offset];
    return result;
}

void
M68B21::write(int offset, uint8_t value) {
    _store[offset] = value;
}
