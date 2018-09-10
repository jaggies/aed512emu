/*
 * unknown.h
 *
 *  Created on: Sep 10, 2018
 *      Author: jmiller
 */

#ifndef UNKNOWN_H_
#define UNKNOWN_H_

#include <iostream>
#include "peripheral.h"

class Unknown: public Peripheral {
    public:
        Unknown(int start, int size, const std::string& name="unknown")
                : Peripheral(start, size, name) { }
        virtual ~Unknown() = default;

        uint8_t read(int offset) override {
            uint8_t result = 0;
            std::cerr << name() << "[0x" << std::hex << start() + offset << "] read "
                    << (int) result << std::endl;
            return 0;
        }

        virtual void write(int offset, uint8_t value) override {
            std::cerr << name() << "[0x" << std::hex << start() + offset << "] = "
                    << (int) value << std::endl;
        }

        void reset() override { }
};

#endif /* UNKNOWN_H_ */
