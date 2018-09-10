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
        M68B21(int start, const std::string& name = "68B21")
                : Peripheral(start, 4, name), _store(6, 0) { }
        virtual ~M68B21() = default;

        // Reads peripheral register at offset
        uint8_t read(int offset) override;

        // Writes peripheral register at offset
        void write(int offset, uint8_t value) override;

        // Reset to initial state
        void reset() override {
            // TODO
        }
    private:
        std::vector<uint8_t> _store;
};

#endif /* M68B21_H_ */
