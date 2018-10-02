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
    public:
        typedef std::function<void(int newValue)> Callback;
        enum PortA {
            PA7 = (1 << 7), PA6 = (1 << 6), PA5 = (1 << 5), PA4 = (1 << 4),
            PA3 = (1 << 3), PA2 = (1 << 2), PA1 = (1 << 1), PA0 = (1 << 0)
        };
        enum PortB {
            PB7 = (1 << 7), PB6 = (1 << 6), PB5 = (1 << 5), PB4 = (1 << 4),
            PB3 = (1 << 3), PB2 = (1 << 2), PB1 = (1 << 1), PB0 = (1 << 0),
        };
        enum ControlA {
            CRA0 = 1<<0, CRA1 = 1<<1, CRA2 = 1<<2, CRA3 = 1<<3,
            CRA4 = 1<<4, CRA5 = 1<<5, CRA6 = 1<<6, CRA7 = 1<<7
        };
        enum ControlB {
            CRB0 = 1<<0, CRB1 = 1<<1, CRB2 = 1<<2, CRB3 = 1<<3,
            CRB4 = 1<<4, CRB5 = 1<<5, CRB6 = 1<<6, CRB7 = 1<<7,
        };
        enum IrqStatusA { CA1 = 0x80, CA2 = 0x40 };
        enum IrqStatusB { CB1 = 0x80, CB2 = 0x40 };
        enum Port { PortA, PortB, ControlA, ControlB, IrqStatusA, IrqStatusB };
        enum Registers { PRA = 0, DDRA = 0, CRA = 1, PRB = 2, DDRB = 2, CRB = 3 };

        M68B21(int start, const std::string& name = "68B21",
                Callback irqA = nullptr, Callback irqB = nullptr,
                Callback aChanged = nullptr, Callback bChanged = nullptr)
                : Peripheral(start, 4, name), _prA(0), _ddrA(0), _crA(0),
                  _prB(0), _ddrB(0), _crB(0), _inA(0), _inB(0), _incA(0), _incB(0),
                  _cbA(aChanged), _cbB(bChanged), _irqA(irqA), _irqB(irqB) { }
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
            _inA = _inB = 0; // TODO: handle static initialization
            _incA = _incB = 0;
        }

        // Returns true if one or more of the given bits is set. The host shouldn't
        // care about ControlA or ControlB.
        bool isSet(Port port, uint8_t data) {
            switch (port) {
                case PortA:
                    return (_inA & data);
                case PortB:
                    return (_inB & data);
                case IrqStatusA:
                    return _crA & data;
                case IrqStatusB:
                    return _crB & data;
                default:
                    return false; // Oops.
            }
        }

        // Sets all bits in data to 1. The host shouldn't be mucking with ControlA or ControlB.
        // Lines should be changed one at a time to ensure IRQ callbacks are issued correctly.
        void set(Port port, uint8_t data);

        // Resets all 1 bits in data. The host shouldn't be mucking with ControlA or ControlB.
        void reset(Port port, uint8_t data);
    private:
        inline bool rising(uint8_t prev, uint8_t cur, uint8_t bit) const {
            return !(prev & bit) && (cur & bit);
        }
        inline bool falling(uint8_t prev, uint8_t cur, uint8_t bit) const {
            return (prev & bit) && !(cur & bit);
        }
        uint8_t get(Port port) const { return port == PortA ? _inA : _inB; }
        uint8_t _prA; // Peripheral Register PortA (when CRA2 is set)
        uint8_t _ddrA; // Data Direction Register PortA. PortA bit value of 0 = input, 1 = output
        uint8_t _crA; // Control Register PortA
        uint8_t _prB; // Peripheral Register PortA (when CRB2 is set)
        uint8_t _ddrB; // Data Direction Register PortB. PortA bit value of 0 = input, 1 = output
        uint8_t _crB; // Control Register PortB
        uint8_t _inA; // Read-only input value, port PortA
        uint8_t _inB; // Read-only input value, port PortB
        uint8_t _incA; // Read-only input value, control port A (bits CA1, CA2)
        uint8_t _incB; // Read-only input value, control port B (bits CB1, CB2)
        Callback _cbA; // Callback invoked when port PortA output changes
        Callback _cbB; // Callback invoked when port PortB output changes
        Callback _irqA;// Callback invoked when CA1 or CA2 input changes
        Callback _irqB;// Callback invoked when CB1 or CB2 input changes
};

#endif /* M68B21_H_ */
