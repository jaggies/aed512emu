/*
 * mapper.h
 *
 *  Created on: Sep 9, 2018
 *      Author: jmiller
 */

#ifndef MAPPER_H_
#define MAPPER_H_

#include <vector>
#include "peripheral.h"

//
// Maps an address to one or more devices. This allows peripherals to overlap,
// which can be used to have backing store for all registers.
//
class Mapper: public Peripheral {
    public:
        Mapper(int start, int size, const std::string& name="") : Peripheral(start, size, name) { }
        virtual ~Mapper() = default;

        // Reads first peripheral register at offset
        uint8_t read(int offset) override {
            for (Peripheral* p : _peripherals) {
                if (offset >= p->start() && offset < p->end()) {
                    const int relativeAddr = offset - p->start();
                    uint8_t result = p->read(relativeAddr);
                    return result; // always return the first one
                }
            }
            return 0;
        }

        // Writes the first peripheral register found at offset
        void write(int offset, uint8_t value) override {
            for (Peripheral* p : _peripherals) {
                if (offset >= p->start() && offset < p->end()) {
                    const int relativeAddr = offset - p->start();
                    p->write(relativeAddr, value);
                    break; // Only write the first one we find
                }
            }
        }

        // Resets all peripherals
        void reset() override {
            for (Peripheral* p : _peripherals) {
                p->reset();
            }
        }

        // Adds a peripheral and mapping
        void add(Peripheral* p) {
            _peripherals.push_back(p);
        }

    private:
        friend std::ostream& operator<<(std::ostream& os, const Mapper& mapper);
        std::vector<Peripheral*> _peripherals;
};

#endif /* MAPPER_H_ */
