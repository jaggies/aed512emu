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
#include <iostream>
#include "peripheral.h"

template <bool debug>
class _Ram: public Peripheral {
    public:
        _Ram(int start, int size, const std::string& name = "RAM")
            : Peripheral(start, size, name), _storage(size, 0) {
            for (size_t i = 0; i < _storage.size(); i++) {
                _storage[i] = i;
            }
        }
        virtual ~_Ram() = default;

        uint8_t read(int offset) override {
            assert(offset >= 0 && offset < _storage.size());
            uint8_t result = _storage[offset];
            if (debug) {
                std::cerr << name() << "[0x" << std::hex << start() + offset << "] read "
                        << (int) result << std::endl;
            }
            return result;
        }

        void write(int offset, uint8_t value) override {
            assert(offset >= 0 && offset < _storage.size());
            if (debug) {
                std::cerr << name() << "[0x" << std::hex << start() + offset << "] write "
                        << (int) value << std::endl;
            }
            _storage[offset] = value;
        }

        void reset() override { } // Typically RAM doesn't reset

        // This bypasses the above read/write to allow host access to RAM
        uint8_t& operator[](int index) { assert(index < _storage.size()); return _storage[index]; }

    protected:
        std::vector<uint8_t> _storage;
};

typedef _Ram<false> Ram;
typedef _Ram<true> RamDebug;

#endif /* RAM_H_ */
