/*
 * aedregs.cpp
 *
 *  Created on: Sep 13, 2018
 *      Author: jmiller
 */

#include <iostream>
#include "aedregs.h"
#include "io.h"

// Reads peripheral register at offset
uint8_t AedRegs::read(int offset) {
    std::cerr << "read " << offset << " ";
    uint8_t result = _values[offset];
    dump(offset, result);
    if (offset == datafl) {
        _values[datafl] ++;
    }
    return result;
}

// Writes peripheral register at offset
void AedRegs::write(int offset, uint8_t value) {
    _values[offset] = value;
    std::cerr << "write " << offset << " " << (int) value << " ";
    dump(offset, value);
}

void AedRegs::dump(int offset, uint8_t value) {
    switch (offset) {
        case hzoom:
        case vzoom:
            std::cerr << "zoom(" << (int) _values[hzoom] << ", " << (int) _values[vzoom] << ")" << std::endl;
        break;

        case capxl:
        case capxh:
        case capyl:
        case capyh:
            std::cerr << "cap(" << (((int)_values[capxh] << 8) | _values[capxl]) << ", "
                << (((int)_values[capyh] << 8) | _values[capyl]) << ")" << std::endl;
        break;

        case vminx:
        case vminy:
            std::cerr << "vmin(" << (int) _values[vminx] << ", " << (int) _values[vminy] << ")" << std::endl;
        break;

        case dmainx:
        case dmainy:
            std::cerr << "dmain(" << (int) _values[dmainx] << ", " << (int) _values[dmainy] << ")" << std::endl;
        break;

        case pxcntl:
        case pxcnth:
            std::cerr << "pxcnt " << (((int) _values[pxcnth] << 8) | _values[pxcntl]) << std::endl;
        break;

        case xscrl:
        case yscrl:
           std::cerr << "scroll(" << (int) _values[xscrl] << ", " << (int) _values[yscrl] << ")" << std::endl;
        break;

        case hstpl:
        case hstph:
            std::cerr << "hstp(" << (int) _values[hstph] << ", " << (int) _values[hstpl] << ")" << std::endl;
        break;

        case datafl:
            std::cerr << "datafl " << (int) _values[datafl] << std::endl;
        break;

        case vmnoi:

        // case miscrd:
        // case rmks0:
        // case rmsk1:
        // case rmsk2:
        // case rmsk3:

        case vminxy:
        case dmainxy:

        case dmanoi:

        case wrmsk:
        case misc0:
        case hstctl:

        case stdvma:

        default:
            std::cerr << "aedregs[" << offset << "] " << (int) value << std::endl;
        break;
    }
}
