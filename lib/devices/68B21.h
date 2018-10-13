/*
 * 68B21.h
 *
 *  Created on: Sep 9, 2018
 *      Author: jmiller
 */

#ifndef M68B21_H_
#define M68B21_H_

#include <vector>
#include <functional>
#include "peripheral.h"

class M68B21 : public Peripheral {
    const uint8_t MODE_MASKA = CRA5 | CRA4;
    const uint8_t MODE_MASKB = CRB5 | CRB4;
    const uint8_t MODE_HANDSHAKEA = CRA5; // CRA5:CRA4 = 10
    const uint8_t MODE_HANDSHAKEB = CRB5; // CRB5:CRB4 = 10
    const uint8_t MODE_DIRECTA = CRA5 | CRA4;
    const uint8_t MODE_DIRECTB = CRB5 | CRB4;
    public:
        enum PortBits {
            PA7 = 1<<7, PA6 = 1<<6, PA5 = 1<<5, PA4 = 1<<4,
            PA3 = 1<<3, PA2 = 1<<2, PA1 = 1<<1, PA0 = 1<<0,
            PB7 = 1<<7, PB6 = 1<<6, PB5 = 1<<5, PB4 = 1<<4,
            PB3 = 1<<3, PB2 = 1<<2, PB1 = 1<<1, PB0 = 1<<0,
        };
        enum ControlBits {
            CRA0 = 1<<0, CRA1 = 1<<1, CRA2 = 1<<2, CRA3 = 1<<3,
            CRA4 = 1<<4, CRA5 = 1<<5, CRA6 = 1<<6, CRA7 = 1<<7,
            CRB0 = 1<<0, CRB1 = 1<<1, CRB2 = 1<<2, CRB3 = 1<<3,
            CRB4 = 1<<4, CRB5 = 1<<5, CRB6 = 1<<6, CRB7 = 1<<7,
        };
        enum IrqStatusBits { CA1 = 0x80, CA2 = 0x40, CB1 = 0x80, CB2 = 0x40 };

        enum Port { InputA, InputB, OutputA, OutputB, ControlA, ControlB };

        // TODO: these should be private.  Currently used for test case.
        enum Registers { PRA = 0, DDRA = PRA, CRA = 1, PRB = 2, DDRB = PRB, CRB = 3 };

        typedef std::function<void(Port port, uint8_t oldvalue, uint8_t newvalue)> RegisterChangedCB;

        M68B21(int start, const std::string& name = "68B21", IRQ irqA = nullptr, IRQ irqB = nullptr,
                RegisterChangedCB changed = nullptr, Signal ca2 = nullptr, Signal cb2 = nullptr)
                :Peripheral(start, 4, name),  _irqA(irqA), _irqB(irqB), _changed(changed),
                 _ca2(ca2), _cb2(cb2) { }
        virtual ~M68B21() = default;

        // Reads peripheral register at offset
        uint8_t read(int offset) override;

        // Writes peripheral register at offset
        void write(int offset, uint8_t value) override;

        // Reset to initial state
        void reset() override {
            _prA = _prB = 0;
            _ddrA = _ddrB = 0;
            _crA = _crB = 0;
            _inA = _inB = 0;
            _inCA = _inCB = 0;
        }

        // Returns true if all of the given bits are set. The host shouldn't
        // care about ControlA or ControlB.
        bool isSet(Port port, uint8_t data) {
            switch (port) {
                case InputA:
                    return (_inA & data) == data;
                case InputB:
                    return (_inB & data) == data;
                case OutputA:
                    return ((_inA & _ddrA) & data) == data;
                case OutputB:
                    return ((_inB & _ddrB) & data) == data;
                case ControlA:
                    return (_crA & data) == data;
                case ControlB:
                    return (_crB & data) == data;
                default:
                    std::cerr << name() << " oops, unrecognized port " << port << std::endl;
                    return false;
            }
        }

        // Sets all bits in data to 1. The host shouldn't be mucking with ControlA or ControlB.
        // Lines should be changed one at a time to ensure IRQ callbacks are issued correctly.
        void set(Port port, uint8_t data);

        // Resets all 1 bits in data. The host shouldn't be mucking with ControlA or ControlB.
        void reset(Port port, uint8_t data);
    private:
        void maybeChangeCA2(bool newValue);
        void maybeChangeCB2(bool newValue);

        uint8_t _prA = 0; // Peripheral Register A (when CRA2 is set)
        uint8_t _ddrA = 0; // Data Direction Register A. A bit value of 0 = input, 1 = output
        uint8_t _crA = 0; // Control Register InputA
        uint8_t _prB = 0; // Peripheral Register B (when CRB2 is set)
        uint8_t _ddrB = 0; // Data Direction Register B. B bit value of 0 = input, 1 = output
        uint8_t _crB = 0; // Control Register B
        uint8_t _inA = 0; // Incoming port A (can only by set by host)
        uint8_t _inB = 0; // Incoming port B (can only by set by host)
        uint8_t _inCA = 0; // Incoming bits CA1, CA2 (can only by set by host)
        uint8_t _inCB = 0; // Incoming bits CB1, CB2 (can only by set by host)
        bool    _outCA2 = false; // Outgoing bit CA2
        bool    _outCB2 = false; // Outgoing bit CB2
        IRQ _irqA;// Callback invoked when CA1 or CA2 input changes
        IRQ _irqB;// Callback invoked when CB1 or CB2 input changes
        RegisterChangedCB _changed; // Callback invoked when any register changes
        Signal  _ca2;   // called when output is configure and CA2 changes
        Signal  _cb2;   // called when output is configured and CB2 changes
};

#endif /* M68B21_H_ */
