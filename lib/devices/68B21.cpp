/*
 * 68B21.cpp
 *
 *  Created on: Sep 9, 2018
 *      Author: jmiller
 */

#include <iostream>
#include <cassert>
#include "coreutil.h"
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
                maybeChangeCA2(0);
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
                if (_changed != nullptr && changed) {
                    _changed(OutputA, _prA, value);
                }
                _prA = value;
            } else {
                _ddrA = value;
            }
        break;
        case CRA: { // CRA
            const uint8_t changed = (_crA ^ (value & 0x3f));
            uint8_t newValue = (_crA & 0xc0) | (value & 0x3f); // two upper bits aren't writeable
            if (_changed != nullptr && changed) {
                _changed(ControlA, _crA, newValue);
            }
            // When CRA4=CRA5=1, then mode is output and CA2 always reflects CRA3 bit
            if ((changed & (MODE_MASKA | CRA3)) && (value & MODE_MASKA) == MODE_DIRECTA)  {
                _outCA2 = value & CRA3;
                if (_ca2) {
                    _ca2(_outCA2);
                }
            }
            _crA = newValue;
        }
        break;
        case PRB: // PRB or DDRB
            if (_crB & CRB2) {
                const bool changed = (_prB ^ value); // TODO: account for write mask
                if (changed && _changed != nullptr) {
                    _changed(OutputB, _prB, value);
                }
                maybeChangeCB2(0);
                _prB = value;
            } else {
                _ddrB = value;
            }
        break;
        case CRB: { // CRB
            const uint8_t changed = (_crB ^ (value & 0x3f));
            uint8_t newValue = (_crB & 0xc0) | (value & 0x3f); // two upper bits aren't writeable
            if (_changed != nullptr && changed) {
                _changed(ControlB, _crB, newValue);
            }
            // When CRB4=CRB5=1, then mode is output and CB2 always reflects CRA3 bit
            if ((changed & (MODE_MASKB | CRB3)) && (value & MODE_MASKB) == MODE_DIRECTB)  {
                _outCB2 = value & CRB3;
                if (_cb2) {
                    _cb2(_outCB2);
                }
            }
            _crB = newValue;
        }
        break;
    }
}

void M68B21::maybeChangeCA2(bool newValue) {
    if ((_crA & MODE_MASKA) == MODE_HANDSHAKEA) {
        if ((_crA & CRA3) == 0) {
            // "Read mode with CA1 Restore"
            if (_ca2 && _outCA2 != newValue) {
                _ca2(newValue);
            }
            _outCA2 = newValue; // active transition on CA1 -> CA2 set. Read of PRA resets.
        } else {
            std::cerr << "Oops, E restore mode\n";
            // "Read mode with E Restore"
            // TODO: have E signal do this
        }
    }
}

void M68B21::maybeChangeCB2(bool newValue) {
    if ((_crB & MODE_MASKB) == MODE_HANDSHAKEB) {
        if ((_crB & CRB3) == 0) {
            // "Write mode with CB1 Restore"
            if (_cb2 && _outCB2 != newValue) {
                _cb2(newValue);
            }
            _outCB2 = newValue; // active transition on CB1 -> CB2 set. Writing PRB resets.
        } else {
            std::cerr << "Oops, E restore mode\n";
            // "Write mode with E Restore"
            // TODO: have E signal do this
        }
    }
}

void M68B21::set(Port port, uint8_t data) {
    switch (port) {
        case InputA:
            _inA |= data;
        break;
        case InputB:
            _inB |= data;
        break;
        case ControlA: {
            data &= (CA1 | CA2); // Only CA1 and CA2 bits can be set
            const bool checkRisingca1 = _crA & CRA1;
            if (checkRisingca1 && rising(_inCA, data, CA1)) { // Falling is done in reset()
                _crA |= CA1; // interrupt!
                if ((data & CA1) && (_crA & CRA0)) { // CA1 IRQ enabled
                    _irqA();
                }
                maybeChangeCA2(1);
            }
            const bool checkRisingca2 = _crA & CRA4;
            if (checkRisingca2 && rising(_inCA, data, CA2)) { // Falling is done in reset()
                _crA |= CA2; // interrupt!
                if ((data & CA2) && (_crA & CRA3)) { // CA2 IRQ enabled
                    _irqA();
                }
            }
            _inCA |= data;
            if (debug) {
                std::cout << name() << " set statusA " << (int) data << " result = " << (int) _inCA << std::endl;
            }
        }
        break;
        case ControlB: {
            // assert((_crB & CRB5) == 0); // E clock config not supported
            data &= (CB1 | CB2); // Only CA1 and CA2 bits can be set
            const bool checkRisingcb1 = _crB & CRB1;
            if (checkRisingcb1 && rising(_inCB, data, CB1)) { // Falling is done in reset()
                _crB |= CB1; // interrupt!
                if ((data & CB1) && (_crB & CRB0)) { // CB1 IRQ enabled
                    _irqB();
                }
                maybeChangeCB2(1);
            }
            const bool checkRisingcb2 = _crB & CRB4;
            if (checkRisingcb2 && rising(_inCB, data, CB2)) { // Falling is done in reset()
                _crB |= CB2; // interrupt!
                if ((data & CB2) && (_crB & CRB3)) { // CB2 IRQ enabled
                    _irqB();
                }
            }
            _inCB |= data;
            if (debug) {
                std::cout << name() << " set statusB " << (int) data << " result = " << (int) _inCB << std::endl;
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
        case ControlA: {
            data &= (CA1 | CA2); // Only CA1 and CA2 bits can be set
            data = ~data; // invert the bits so we can simply AND them to clear
            const bool checkFallingca1 = !(_crA & CRA1);
            if (checkFallingca1 && falling(_inCA, data, CA1)) { // Rising is done in set()
                _crA |= CA1; // interrupt!
                if (_crA & CRA0) { // CA1 IRQ enabled
                    _irqA();
                }
                maybeChangeCA2(1);
            }
            const bool checkFallingca2 = !(_crA & CRA4);
            if (checkFallingca2 && falling(_inCA, data, CA2)) { // Rising is done in set()
                _crA |= CA2; // interrupt!
                if (_crA & CRA3) { // CA2 IRQ enabled
                    _irqA();
                }
            }
            _inCA &= data;
            if (debug) {
                std::cout << name() << " reset statusA " << (int) ~data << " result = " << (int) _inCA << std::endl;
            }
        }
        break;
        case ControlB: {
            data &= (CB1 | CB2); // Only CB1 and CB2 bits can be reset
            data = ~data; // invert the bits so we can simply AND them to clear
            const bool checkFallingcb1 = !(_crB & CRB1);
            if (checkFallingcb1 && falling(_inCB, data, CB1)) { // Rising is done in set()
                _crB |= CB1;
                if (_crB & CRB0) { // CB1 IRQ enabled
                    _irqB();
                }
                maybeChangeCB2(1);
            }
            const bool checkFallingcb2 = !(_crB & CRB4);
            if (checkFallingcb2 && falling(_inCB, data, CB2)) { // Rising is done in set()
                _crB |= CB2;
                if (_crB & CRB3) { // CB2 IRQ enabled
                    _irqB();
                }
            }
            _inCB &= data;
            if (debug) {
                std::cout << name() << " reset statusB " << (int) ~data << " result = " << (int) _inCB << std::endl;
            }
        }
        break;
        default:
            // Oops.
        break;
    }
}
