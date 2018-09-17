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
        Rom(int start, const std::vector<uint8_t>& init, bool allowWrites = false)
                : Ram(start, init.size(), "ROM"), _allowWrites(allowWrites) {
            assert(init.size() == _storage.size());
            std::copy(init.begin(), init.end(), _storage.begin());
        }
        virtual ~Rom() = default;

        void write(int offset, uint8_t value) override {
            if (_allowWrites) {
                Ram::write(offset, value);
            } else {
                std::cerr << "Ooops, tried to write to ROM: offset = " << offset << std::endl;
            }
        }
    private:
        bool _allowWrites;
};

#endif /* ROM_H_ */
