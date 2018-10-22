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

// static const char* misc0bits[] = { "zrm", "yzs", "wob", "bw", "dma", "xup", "yup", "pxen" };

static bool debug = false;

// Reads peripheral register at offset
uint8_t AedRegs::read(int offset) {
    uint16_t& x = *(uint16_t *) &_storage[capxl]; // X and Y CAP
    uint16_t& y = *(uint16_t *) &_storage[capyl];
    const int dX = (_storage[misc0] & X_UD) ? 1 : -1; // X and Y increments, if used
    const int dY = (_storage[misc0] & Y_UD) ? 1 : -1;
    uint8_t result;
    switch (offset) {
        case dmanoi:
        case vmnoi:
            result = pixel(x, y);
        break;

        case dmainx:
        case vminx:
            result = pixel(x, y);
            x += dX;
        break;

        case dmainy:
        case vminy:
            result = pixel(x, y);
            y += dY;
        break;

        case dmainxy:
        case vminxy:
            result = pixel(x, y);
            x += dX;
            y += dY;
        break;

        default:
            result = _storage[offset];
        break;
    }
    return result;
}

// Writes peripheral register at offset
void AedRegs::write(int offset, uint8_t value) {
    const uint16_t pixcount = 1; // TODO: update for DMA
    int dX = (_storage[misc0] & X_UD) ? 1 : -1;
    int dY = (_storage[misc0] & Y_UD) ? 1 : -1;
    switch (offset) {
        case dmanoi:
        case vmnoi:
            doVideoUpdate(0, 0, value, pixcount);
            break;

        case dmainx:
        case vminx:
            doVideoUpdate(dX, 0, value, pixcount);
            break;

        case dmainy:
        case vminy:
            doVideoUpdate(0, dY, value, pixcount);
            break;

        case dmainxy:
        case vminxy:
            doVideoUpdate(dX, dY, value, pixcount);
            break;

        case xscrl:
        case yscrl:
            if (debug) {
                std::cerr << "scroll(" << (int) _storage[xscrl] << "," << (int) _storage[yscrl] << ")" << std::endl;
            }
        break;

        case vzoom:
        case unkn0:
        case unkn1:
        case hzoom:
            if (!(_storage[misc0] & 1)) {
                return; // return w/o storing it.
            } else {
                if (debug) std::cerr << "zoom[" << offset << "] = " << (int) value << std::endl;
            }
        break;

        case pxcnth:
            if (_pixcntoflo && ((_storage[offset] ^ value) & 0x0f)) {
                // This is the difference between A/B versions of the AED512
                _pixcntoflo((value & 0x0f) == 0x0f); // Terminal Count (TC) from 74ls191 U164
            }
        break;

        default:
        break;
    }
    _storage[offset] = value;
}

uint8_t& AedRegs::pixel(int x, int y) {
    x &= DISPLAY_WIDTH-1;
    y &= DISPLAY_HEIGHT-1;
    return _videoMemory[y*DISPLAY_WIDTH + x];
}

void AedRegs::doVideoUpdate(int dX, int dY, uint8_t color, uint16_t count) {
    uint16_t& x = *(uint16_t *) &_storage[capxl];  // TODO: fix for big endian machines
    uint16_t& y = *(uint16_t *) &_storage[capyl];
    const uint8_t mask = _storage[wrmsk];
    while (count--) {
        uint8_t &p = pixel(x,y);
        p = (p & ~mask) | (color & mask);
        x += dX;
        y += dY;
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
