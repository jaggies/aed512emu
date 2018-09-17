/*
 * genericperipheral.h
 *
 *  Created on: Sep 15, 2018
 *      Author: jmiller
 */

#ifndef GENERICPERIPHERAL_H_
#define GENERICPERIPHERAL_H_

#include <functional>
#include "peripheral.h"

class Generic: public Peripheral {
    public:
        Generic(int start, int size,
                const std::function<uint8_t(int)>& read,
                const std::function<void(int offset, uint8_t value)>& write,
                const std::string& name="generic") : Peripheral(start, size, name),
                    _read(read), _write(write) { }
        virtual ~Generic() = default;

        // Reads peripheral register at offset
        uint8_t read(int offset) override {
            return _read(offset);
        }

        // Writes peripheral register at offset
        void write(int offset, uint8_t value) override {
            _write(offset, value);
        }

        void reset() override {

        }
    private:
        std::function<uint8_t(int)> _read;
        std::function<void(int offset, uint8_t value)> _write;
};

#endif /* GENERICPERIPHERAL_H_ */
