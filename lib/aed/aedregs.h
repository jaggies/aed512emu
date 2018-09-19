/*
 * aedregs.h
 *
 *  Created on: Sep 13, 2018
 *      Author: jmiller
 */

#ifndef AEDREGS_H_
#define AEDREGS_H_

#include <vector>
#include <iostream>
#include "peripheral.h"

class AedRegs: public Peripheral {
    #if defined(AED767) || defined(AED1024)
    static const size_t DISPLAY_WIDTH = 1024;
    static const size_t DISPLAY_HEIGHT = 1024;
    #else
    static const size_t DISPLAY_WIDTH = 512;
    static const size_t DISPLAY_HEIGHT = 512;
    #endif

    public:
        AedRegs(int start, int size, const std::string& name="")
            : Peripheral(start, size, name), _storage(size, 0),
              _videoMemory(DISPLAY_WIDTH * DISPLAY_HEIGHT * sizeof(uint8_t), 0x00) {
        }
        virtual ~AedRegs() = default;

        // Reads peripheral register at offset
        uint8_t read(int offset) override;

        // Writes peripheral register at offset
        void write(int offset, uint8_t value) override;

        // Hardware reset initializes all registers to peripheral-defined value
        void reset() override {
            for (size_t i = 0; i < _storage.size(); i++) {
                _storage[i] = 0;
            }
        }

        const size_t getDisplayWidth() const { return DISPLAY_WIDTH; }

        const size_t getDisplayHeight() const { return DISPLAY_HEIGHT; }

        const std::vector<uint8_t>& getVideoMemory() const { return _videoMemory; }

        uint8_t& pixel(int x, int y);

    private:
        void doVideoUpdate(int dX, int dY, uint8_t color, uint16_t count);
        void dump(int offset, uint8_t value);
        std::vector<uint8_t> _storage;
        std::vector<uint8_t> _videoMemory;
};

#endif /* AEDREGS_H_ */
