/*
 * aedregs.h
 *
 *  Created on: Sep 13, 2018
 *      Author: jmiller
 */

#ifndef AEDREGS_H_
#define AEDREGS_H_

#include <vector>
#include <iostream>
#include "peripheral.h"

class AedRegs: public Peripheral {
    public:
        AedRegs(int start, int size, const std::string& name="")
            : Peripheral(start, size, name), _values(size, 0) { }
        virtual ~AedRegs() = default;

        // Reads peripheral register at offset
        uint8_t read(int offset) override;

        // Writes peripheral register at offset
        void write(int offset, uint8_t value) override;

        // Hardware reset initializes all registers to peripheral-defined value
        void reset() override {
            for (size_t i = 0; i < _values.size(); i++) {
                _values[i] = 0;
            }
        }

    private:
        void dump(int offset, uint8_t value);
        std::vector<uint8_t> _values;
};

#endif /* AEDREGS_H_ */
