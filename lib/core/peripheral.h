/*
 * peripheral.h
 *
 *  Created on: Sep 9, 2018
 *      Author: jmiller
 */

#ifndef PERIPHERAL_H_
#define PERIPHERAL_H_

#include <functional>
#include <cstdlib>
#include <string>

class Peripheral {
    public:
        typedef std::function<void(void)> IRQ;
        typedef std::function<void(bool set)> Signal;

        Peripheral(int start, int size, const std::string& name="")
            : _start(start), _size(size), _name(name) { }
        virtual ~Peripheral() = default;

        // Returns the start address of this peripheral
        int start() const { return _start; }

        // Returns the end address of this peripheral
        int end() const { return _start + _size; }

        // Returns the size of this peripheral in bytes
        int size() const { return _size; }

        // Returns true of the offset maps to this device
        inline bool maps(int offset) const {
           return offset >= _start && offset < _start + _size;
        }

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
