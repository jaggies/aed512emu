/*
 * rom.h
 *
 *  Created on: Sep 10, 2018
 *      Author: jmiller
 */

#ifndef ROM_H_
#define ROM_H_

#include <iostream>
#include <cassert>
#include "ram.h"

class Rom: public Ram {
    public:
        Rom(int start, const std::vector<uint8_t>& init) : Ram(start, init.size(), "ROM") {
            assert(init.size() == getBuffer().size());
            std::copy(init.begin(), init.end(), getBuffer().begin());
        }
        virtual ~Rom() = default;

        void write(int offset, uint8_t value) override {
            std::cerr << "Ooops, tried to write to ROM: offset = " << offset << std::endl;
        }
};

#endif /* ROM_H_ */
