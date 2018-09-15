/*
 * fuzz.h
 *
 *  Created on: Sep 10, 2018
 *      Author: jmiller
 */

#ifndef FUZZ_H_
#define FUZZ_H_

#include <vector>
#include "peripheral.h"

// A class for fuzzing peripherals to allow the device to boot.
// It returns random data to allow the code to eventually get what it's
// looking for.
class Fuzz: public Peripheral {
    public:
        Fuzz(int start, int size, const std::string& name = "RAM")
                : Peripheral(start, size, name), _storage(size, 0) { }
        virtual ~Fuzz() = default;

        // Reads peripheral register at offset
        uint8_t read(int offset) override { return random(); } //_storage[offset]++; }

        // Writes peripheral register at offset
        void write(int offset, uint8_t value) override { } //_storage[offset] = value; }

        // Reset to initial state
        void reset() override {
            // TODO
        }
    private:
        std::vector<uint8_t> _storage;
};

#endif /* FUZZ_H_ */
