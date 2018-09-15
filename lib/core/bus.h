/*
 * BUS.h
 *
 *  Created on: Sep 8, 2018
 *      Author: jmiller
 */

#ifndef BUS_H
#define BUS_H

class BUS {
    public:
        BUS() = default;
        virtual ~BUS() = default;
        virtual uint8_t read(int addr) = 0;
        virtual void write(int addr, uint8_t value) = 0;
};

#endif // BUS_H
