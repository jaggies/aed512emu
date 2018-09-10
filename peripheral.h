/*
 * peripheral.h
 *
 *  Created on: Sep 9, 2018
 *      Author: jmiller
 */

#ifndef PERIPHERAL_H_
#define PERIPHERAL_H_

#include <cstdlib>
#include <string>

class Peripheral {
    public:
        Peripheral(int start, int size, const std::string& name="")
            : _start(start), _size(size), _name(name) { }
        virtual ~Peripheral() = default;

        int start() const { return _start; }
        int end() const { return _start + _size; }
        int size() const { return _size; }

        // Reads peripheral register at offset
        virtual uint8_t read(int offset) = 0;

        // Writes peripheral register at offset
        virtual void write(int offset, uint8_t value) = 0;

        // Hardware reset initializes all registers to peripheral-defined value
        virtual void reset() = 0;

        const std::string& name() const { return _name; }
    private:
        int _start;
        int _size;
        std::string _name;
};

#endif /* PERIPHERAL_H_ */
