/*
 * 68B21.cpp
 *
 *  Created on: Sep 9, 2018
 *      Author: jmiller
 */

#include <iostream>
#include <cassert>
#include "util.h"
#include "68B21.h"

static bool debug = false;

uint8_t M68B21::read(int offset) {
    assert(offset < 4);
    uint8_t result = 0;
    switch (offset) {
        case PRA: // PRA or DDRA
            if (_crA & CRA2) {
                result = (_inA & ~_ddrA) | (_prA & _ddrA);
                _crA &= 0x3f; // read of peripheral register resets IRQ bits
            } else {
                result = _ddrA;
            }
            break;
        case CRA: // CRA
            result = _crA;
            if (debug) std::cerr << name() << ":CRA read -> " << (int) result << std::endl;
            break;
        case PRB: // PRB or DDRB
            if (_crB & CRB2) {
                result = (_inB & ~_ddrB) | (_prB & _ddrB);
                _crB &= 0x3f; // read of peripheral register resets IRQ bits
            } else {
                result = _ddrB;
            }
            break;
        case CRB: // CRB
            result = _crB;
            if (debug) std::cerr << name() << ":CRB read -> " << (int) result << std::endl;
            break;
    }
    return result;
}

void M68B21::write(int offset, uint8_t value) {
    assert(offset < 4);
    switch (offset) {
        case PRA: // PRA or DDRA
            if (_crA & CRA2) {
                const bool changed = (_prA ^ value); // TODO: account for write mask
                if (_registerChanged != nullptr && changed) {
                    _registerChanged(OutputA, _prA, value);
                }
                _prA = value;
            } else {
                _ddrA = value;
            }
        break;
        case CRA: { // CRA
            if (debug) std::cerr << std::hex << name() << " CRA write: " << (int) value << std::endl;
            _crA = (_crA & 0xc0) | (value & 0x3f); // two upper bits aren't writeable
            if (debug && (_crA & (CRA5 | CRA4)) != (CRA5 | CRA4)) { // if not output mode for CA2
                std::cerr << name() << ": CRA E clock not supported!" << std::endl;
            }
        }
        break;
        case PRB: // PRB or DDRB
            if (_crB & CRB2) {
                const bool changed = (_prB ^ value); // TODO: account for write mask
                if (changed && _registerChanged != nullptr) {
                    _registerChanged(OutputB, _prB, value);
                }
                _prB = value;
            } else {
                _ddrB = value;
            }
        break;
        case CRB: { // CRB
            if (debug) std::cerr << std::hex << name() << " CRB write: " << (int) value << std::endl;
            _crB = (_crB & 0xc0) | (value & 0x3f); // two upper bits aren't writeable
            if (debug && (_crB & (CRB5 | CRB4)) != (CRB5 | CRB4)) { // if not output mode for CB2
                std::cerr << name() << ": CRB E clock not supported!" << std::endl;
            }
        }
        break;
    }
}

// Notes:
// CA1, CB1:
//      IRQ_en = CRx0 (where x = A or B)
//      IRQ_edge = CRx1 ? rising : falling
// CA2, CB2: (when CRx5 = 0):
//      IRQ_en = CRx3 (where x = A or B)
//      IRQ_edge = CRx4 ? rising : falling
// CRx5 = 1 -> follow E clock (unimplemented)
void M68B21::set(Port port, uint8_t data) {
    switch (port) {
        case InputA:
            _inA |= data;
        break;
        case InputB:
            _inB |= data;
        break;
        case IrqStatusA: {
            // assert((_crA & CRA5) == 0); // E clock config not supported
            data &= (CA1 | CA2); // Only CA1 and CA2 bits can be set
            const bool checkRisingca1 = _crA & CRA1;
            if (checkRisingca1 && rising(_incA, data, CA1)) { // Falling is done in reset()
                _crA |= CA1; // interrupt!
                if ((data & CA1) && (_crA & CRA0)) { // CA1 IRQ enabled
                    _irqA();
                }
            }
            const bool checkRisingca2 = _crA & CRA4;
            if (checkRisingca2 && rising(_incA, data, CA2)) { // Falling is done in reset()
                _crA |= CA2; // interrupt!
                if ((data & CA2) && (_crA & CRA3)) { // CA2 IRQ enabled
                    _irqA();
                }
            }
            _incA |= data;
            if (debug) {
                std::cout << name() << " set statusA " << (int) data << " result = " << (int) _incA << std::endl;
            }
        }
        break;
        case IrqStatusB: {
            // assert((_crB & CRB5) == 0); // E clock config not supported
            data &= (CB1 | CB2); // Only CA1 and CA2 bits can be set
            const bool checkRisingcb1 = _crB & CRB1;
            if (checkRisingcb1 && rising(_incB, data, CB1)) { // Falling is done in reset()
                _crB |= CB1; // interrupt!
                if ((data & CB1) && (_crB & CRB0)) { // CB1 IRQ enabled
                    _irqB();
                }
            }
            const bool checkRisingcb2 = _crB & CRB4;
            if (checkRisingcb2 && rising(_incB, data, CB2)) { // Falling is done in reset()
                _crB |= CB2; // interrupt!
                if ((data & CB2) && (_crB & CRB3)) { // CB2 IRQ enabled
                    _irqB();
                }
            }
            _incB |= data;
            if (debug) {
                std::cout << name() << " set statusB " << (int) data << " result = " << (int) _incB << std::endl;
            }
        }
        break;
        default:
            // Oops.
        break;
    }
}

void M68B21::reset(Port port, uint8_t data) {
    switch (port) {
        case InputA:
            _inA &= ~data;
        break;
        case InputB:
            _inB &= ~data;
        break;
        case IrqStatusA: {
            assert((_crA & CRA5) == 0); // E clock config not supported
            data &= (CA1 | CA2); // Only CA1 and CA2 bits can be set
            data = ~data; // invert the bits so we can simply AND them to clear
            const bool checkFallingca1 = !(_crA & CRA1);
            if (checkFallingca1 && falling(_incA, data, CA1)) { // Rising is done in set()
                _crA |= CA1; // interrupt!
                if (_crA & CRA0) { // CA1 IRQ enabled
                    _irqA();
                }
            }
            const bool checkFallingca2 = !(_crA & CRA4);
            if (checkFallingca2 && falling(_incA, data, CA2)) { // Rising is done in set()
                _crA |= CA2; // interrupt!
                if (_crA & CRA3) { // CA2 IRQ enabled
                    _irqA();
                }
            }
            _incA &= data;
            if (debug) {
                std::cout << name() << " reset statusA " << (int) ~data << " result = " << (int) _incA << std::endl;
            }
        }
        break;
        case IrqStatusB: {
            assert((_crA & CRA5) == 0); // E clock config not supported
            data &= (CB1 | CB2); // Only CB1 and CB2 bits can be reset
            data = ~data; // invert the bits so we can simply AND them to clear
            const bool checkFallingcb1 = !(_crB & CRB1);
            if (checkFallingcb1 && falling(_incB, data, CB1)) { // Rising is done in set()
                _crB |= CB1;
                if (_crB & CRB0) { // CB1 IRQ enabled
                    _irqB();
                }
            }
            const bool checkFallingcb2 = !(_crB & CRB4);
            if (checkFallingcb2 && falling(_incB, data, CB2)) { // Rising is done in set()
                _crB |= CB2;
                if (_crB & CRB3) { // CB2 IRQ enabled
                    _irqB();
                }
            }
            _incB &= data;
            if (debug) {
                std::cout << name() << " reset statusB " << (int) ~data << " result = " << (int) _incB << std::endl;
            }
        }
        break;
        default:
            // Oops.
        break;
    }
}
