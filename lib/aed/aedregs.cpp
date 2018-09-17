/*
 * aedregs.cpp
 *
 *  Created on: Sep 13, 2018
 *      Author: jmiller
 */

#include <iostream>
#include <cassert>
#include "aedregs.h"
#include "io.h"

// Reads peripheral register at offset
uint8_t AedRegs::read(int offset) {
    uint8_t result = _storage[offset];
    //std::cerr << "read(" << offset << ")->" << (int) result << " ";
    //dump(offset, result);
    return result;
}

void AedRegs::doVideoUpdate(int dX, int dY, uint8_t color, uint16_t count) {
    // TODO: This only works on a little endian machines!!!
    uint16_t& x = *(uint16_t *) &_storage[capxl];
    uint16_t& y = *(uint16_t *) &_storage[capyl];
    while (count--) {
        x += dX;
        y += dY;
//        assert(x < DISPLAY_WIDTH);
        assert(y < DISPLAY_HEIGHT);
        std::cerr << "Write pixel (" << x << ", " << y << ") inc[" << dX << "," << dY << "]"
                " = " << (int) color << std::endl;
        _videoMemory[y*DISPLAY_WIDTH + x] = color;
    }
}

// Writes peripheral register at offset
void AedRegs::write(int offset, uint8_t value) {
    _storage[offset] = value;
//    std::cerr << "write(" << offset << ", " << (int) value << ")" << std::endl;
    int dX = 0;
    int dY = 0;
    uint16_t pixcount = 1; // TODO: update for DMA
    switch (offset) {
        case dmanoi: std::cerr << "*** DMANOI ***\n"; // TODO
        case vmnoi:
            dX = dY = 0;
            doVideoUpdate(dX, dY, value, pixcount);
            break;
        case dmainx: std::cerr << "*** pixel DMAINX ***\n"; // TODO
        case vminx:
            dX = (_storage[misc0] & X_UD) ? 1 : -1;
            doVideoUpdate(dX, dY, value, pixcount);
            break;
        case dmainy: std::cerr << "*** pixel DMAINY ***\n"; // TODO
        case vminy:
            dY = (_storage[misc0] & Y_UD) ? 1 : -1;
            doVideoUpdate(dX, dY, value, pixcount);
            break;
        case dmainxy: std::cerr << "*** pixel DMAINXY ***\n"; // TODO
        case vminxy:
            dX = (_storage[misc0] & X_UD) ? 1 : -1;
            dY = (_storage[misc0] & Y_UD) ? 1 : -1;
            doVideoUpdate(dX, dY, value, pixcount);
            break;
    }
}

void AedRegs::dump(int offset, uint8_t value) {
    switch (offset) {
        case hzoom:
        case vzoom:
            std::cerr << "zoom(" << (int) _storage[hzoom] << ", " << (int) _storage[vzoom] << ")"
                    << std::endl;
        break;

        case capxl:
        case capxh:
        case capyl:
        case capyh:
            std::cerr << "cap(" << (((int) _storage[capxh] << 8) | _storage[capxl]) << ", "
                    << (((int) _storage[capyh] << 8) | _storage[capyl]) << ")" << std::endl;
        break;

        case vminx:
        case vminy:
            std::cerr << "vmin(" << (int) _storage[vminx] << ", " << (int) _storage[vminy] << ")"
                    << std::endl;
        break;

        case dmainx:
        case dmainy:
            std::cerr << "dmain(" << (int) _storage[dmainx] << ", " << (int) _storage[dmainy] << ")"
                    << std::endl;
        break;

        case pxcntl:
        case pxcnth:
            // TODO: assert _pia1.CA2+ if this count is related to PIXCNTOFLO
            std::cerr << "pxcnt " << (((int) _storage[pxcnth] << 8) | _storage[pxcntl]) << std::endl;
        break;

        case xscrl:
        case yscrl:
            std::cerr << "scroll(" << (int) _storage[xscrl] << ", " << (int) _storage[yscrl] << ")"
                    << std::endl;
        break;

        case hstpl:
        case hstph:
            std::cerr << "hstp(" << (int) _storage[hstph] << ", " << (int) _storage[hstpl] << ")"
                    << std::endl;
        break;

        case datafl:
            std::cerr << "datafl " << (int) _storage[datafl] << std::endl;
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
