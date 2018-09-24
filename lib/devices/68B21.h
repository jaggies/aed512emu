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
        enum Line {
            CA1, CA2, CB1, CB2
        };
        typedef std::function<void(int newValue)> Callback;

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
                case CA1: CRA &= 0x80; break;
                case CA2: CRA &= 0x40; break;
                case CB1: CRB &= 0x80; break;
                case CB2: CRB &= 0x40; break;
            }
        }

        void setA(uint8_t data) { inA = data; }
        void setB(uint8_t data) { inB = data; }
        uint8_t getA() const { return inA; }
        uint8_t getB() const { return inB; }

    private:
        uint8_t PRA; // Peripheral Register A (when CRA2 is set)
        uint8_t DDRA; // Data Direction Register A. A bit value of 0 = input, 1 = output
        uint8_t CRA; // Control Register A
        uint8_t PRB; // Peripheral Register A (when CRB2 is set)
        uint8_t DDRB; // Data Direction Register B. A bit value of 0 = input, 1 = output
        uint8_t CRB; // Control Register B
        uint8_t inA; // Read-only input value, port A
        uint8_t inB; // Read-only input value, port B
        Callback cbA; // Callback invoked when port A output changes
        Callback cbB; // Callback invoked when port B output changes
        enum ControlBits {
            CRA0 = 1<<0,
            CRA1 = 1<<1,
            CRA2 = 1<<2,
            CRA3 = 1<<3,
            CRA4 = 1<<4,
            CRA5 = 1<<5,
            CRA6 = 1<<6,
            CRA7 = 1<<7,
            CRB0 = 1<<0,
            CRB1 = 1<<1,
            CRB2 = 1<<2,
            CRB3 = 1<<3,
            CRB4 = 1<<4,
            CRB5 = 1<<5,
            CRB6 = 1<<6,
            CRB7 = 1<<7,
        };
};

#endif /* M68B21_H_ */
