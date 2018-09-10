/*
 * aedbus.h
 *
 *  Created on: Sep 8, 2018
 *      Author: jmiller
 */

#ifndef AEDBUS_H
#define AEDBUS_H

#include <vector>
#include <cinttypes>
#include "bus.h"
#include "mapper.h"

class AedBus : public BUS {
    public:
        AedBus();
        virtual ~AedBus();
        uint8_t read(int addr) override { return _mapper.read(addr); }
        void write(int addr, unsigned char value) override { _mapper.write(addr, value); }
    private:
        Mapper _mapper;
        std::vector<uint8_t> _videoMemory;
};

#endif // AEDBUS_H
