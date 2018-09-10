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
            for (Peripheral* p : map(offset)) {
                const int relativeAddr = offset - p->start();
                uint8_t result = p->read(relativeAddr);
                #ifdef DEBUG
                std::cerr << "read " << p->name() << "[" << relativeAddr << "] = "
                        << (int) result << std::endl;
                #endif
                return result;
            }
            return 0;
        }

        // Writes all peripheral registers at offset
        void write(int offset, uint8_t value) override {
            for (Peripheral* p : map(offset)) {
                const int relativeAddr = offset - p->start();
                #ifdef DEBUG
                std::cerr << "write " << p->name() << "[" << relativeAddr << "] = "
                        << (int) value << std::endl;
                #endif
                p->write(relativeAddr, value);
                break; // Only write the first one we find
            }
        }

        void reset() override {
            for (Peripheral* p : _peripherals) {
                p->reset();
            }
        }

        void add(Peripheral* p) {
            _peripherals.push_back(p);
        }

    private:
        friend std::ostream& operator<<(std::ostream& os, const Mapper& mapper);
        std::vector<Peripheral*> map(int addr) const {
            std::vector<Peripheral*> result;
            for (Peripheral* p : _peripherals) {
                if (addr >= p->start() && addr < p->end()) {
                    result.push_back(p);
                }
            }
            return result;
        }
        std::vector<Peripheral*> _peripherals;
};

#endif /* MAPPER_H_ */
