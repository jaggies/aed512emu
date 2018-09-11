/*
 * m68b50.h
 *
 *  Created on: Sep 10, 2018
 *      Author: jmiller
 */

#ifndef M68B50_H_
#define M68B50_H_

#include <vector>
#include "peripheral.h"

class M68B50: public Peripheral {
    public:
        M68B50(int start, const std::string& name = "68B21")
                : Peripheral(start, 2, name), _store(2, 0) { };
        virtual ~M68B50() = default;

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

#endif /* M68B50_H_ */
