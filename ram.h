/*
 * ram.h
 *
 *  Created on: Sep 10, 2018
 *      Author: jmiller
 */

#ifndef RAM_H_
#define RAM_H_

#include <vector>
#include <cassert>
#include "peripheral.h"

class Ram: public Peripheral {
    public:
        Ram(int start, int size, const std::string& name = "RAM")
            : Peripheral(start, size, name), _ram(size, 0) { }
        virtual ~Ram() = default;

        uint8_t read(int offset) override {
            assert(offset >= 0 && offset < _ram.size());
            return _ram[offset];
        }

        void write(int offset, uint8_t value) override {
            assert(offset >= 0 && offset < _ram.size());
            _ram[offset] = value;
        }

        void reset() override { } // Typically RAM doesn't reset
    protected:
        std::vector<uint8_t>& getBuffer() { return _ram; }

    private:
        std::vector<uint8_t> _ram;
};

#endif /* RAM_H_ */
