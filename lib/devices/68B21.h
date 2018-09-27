/*
 * 68B21.h
 *
 *  Created on: Sep 9, 2018
 *      Author: jmiller
 */

#ifndef M68B21_H_
#define M68B21_H_

#include <vector>
#include "peripheral.h"

class M68B21 : public Peripheral {
    public:
        typedef std::function<void(int newValue)> Callback;
        enum PortBits {
            PA7 = (1 << 7), PA6 = (1 << 6), PA5 = (1 << 5), PA4 = (1 << 4),
            PA3 = (1 << 3), PA2 = (1 << 2), PA1 = (1 << 1), PA0 = (1 << 0),
            PB7 = (1 << 7), PB6 = (1 << 6), PB5 = (1 << 5), PB4 = (1 << 4),
            PB3 = (1 << 3), PB2 = (1 << 2), PB1 = (1 << 1), PB0 = (1 << 0),
        };
        enum ControlBits {
            CRA0 = 1<<0, CRA1 = 1<<1, CRA2 = 1<<2, CRA3 = 1<<3,
            CRA4 = 1<<4, CRA5 = 1<<5, CRA6 = 1<<6, CRA7 = 1<<7,
            CRB0 = 1<<0, CRB1 = 1<<1, CRB2 = 1<<2, CRB3 = 1<<3,
            CRB4 = 1<<4, CRB5 = 1<<5, CRB6 = 1<<6, CRB7 = 1<<7,
        };
        enum Line { CA1, CA2, CB1, CB2 };
        enum Port { PortA = 0, PortB };

        M68B21(int start, const std::string& name = "68B21", uint8_t aInit = 0, uint8_t bInit = 0,
                Callback aChanged = nullptr, Callback bChanged = nullptr)
                : Peripheral(start, 4, name),
                  PRA(0), DDRA(0), CRA(0), PRB(0), DDRB(0), CRB(0), inA(aInit), inB(bInit),
                  cbA(aChanged), cbB(bChanged)
        { }
        virtual ~M68B21() = default;

        // Reads peripheral register at offset
        uint8_t read(int offset) override;

        // Writes peripheral register at offset
        void write(int offset, uint8_t value) override;

        // Reset to initial state
        void reset() override {
            PRA = inA;
            PRB = inB;
            DDRA = DDRB = CRA = CRB = 0;
        }

        // Check if the line is asserted
        bool isAssertedLine(Line line) {
            switch(line) {
                case CA1: return CRA & 0x80; break;
                case CA2: return CRA & 0x40; break;
                case CB1: return CRB & 0x80; break;
                case CB2: return CRB & 0x40; break;
                default: return false;
            }
        }

        // Assert IRQ line
        void assertLine(Line line) {
            switch(line) {
                case CA1: CRA |= 0x80; break;
                case CA2: CRA |= 0x40; break;
                case CB1: CRB |= 0x80; break;
                case CB2: CRB |= 0x40; break;
            }
        }
        // Assert IRQ line
        void deassertLine(Line line) {
            switch(line) {
                case CA1: CRA &= ~0x80; break;
                case CA2: CRA &= ~0x40; break;
                case CB1: CRB &= ~0x80; break;
                case CB2: CRB &= ~0x40; break;
            }
        }

        bool isSet(Port port, uint8_t data) {
            return port == PortA ? (inA & data) : (inB & data);
        }

        // Sets all bits in data to 1
        void set(Port port, uint8_t data) {
            if (port == PortA) {
                inA |= data;
            } else {
                inB |= data;
            }
        }

        // Resets all 1 bits in data
        void reset(Port port, uint8_t data) {
            if (port == PortA) {
                inA &= ~data;
            } else {
                inB &= ~data;
            }
        }

    private:
        uint8_t get(Port port) const { return port == PortA ? inA : inB; }
        uint8_t PRA; // Peripheral Register PortA (when CRA2 is set)
        uint8_t DDRA; // Data Direction Register PortA. PortA bit value of 0 = input, 1 = output
        uint8_t CRA; // Control Register PortA
        uint8_t PRB; // Peripheral Register PortA (when CRB2 is set)
        uint8_t DDRB; // Data Direction Register PortB. PortA bit value of 0 = input, 1 = output
        uint8_t CRB; // Control Register PortB
        uint8_t inA; // Read-only input value, port PortA
        uint8_t inB; // Read-only input value, port PortB
        Callback cbA; // Callback invoked when port PortA output changes
        Callback cbB; // Callback invoked when port PortB output changes
};

#endif /* M68B21_H_ */
