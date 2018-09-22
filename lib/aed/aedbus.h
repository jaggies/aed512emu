/*
 * aedbus.h
 *
 *  Created on: Sep 8, 2018
 *      Author: jmiller
 */

#ifndef AEDBUS_H
#define AEDBUS_H

#include <vector>
#include <queue>
#include <cinttypes>
#include <sys/time.h>
#include "bus.h"
#include "mapper.h"
#include "ram.h"
#include "rom.h"
#include "68B21.h"
#include "68B50.h"
#include "aedregs.h"

class AedBus : public BUS {
    #define SECS2USECS(a) ((a)*1000000)
    public:
        AedBus();
        virtual ~AedBus();
        uint8_t read(int addr) override { return _mapper.read(addr); }
        void write(int addr, unsigned char value) override { _mapper.write(addr, value); }

        void reset() {
            _mapper.reset();
        }

        // Handle video-related timing and return 'true' if IRQ needs to happen
        // The time_us parameter is microseconds from the CPU perspective, not host time.
        bool doVideo(uint64_t time_us);

        // Looks for data from serial ports. Returns true if IRQ needs to happen.
        // TODO: Have this automatically invoke serial callback when requested by CPU write
        bool doSerial();

        // Copies string to FIFO for handling in doSerial()
        void send(const std::string& string) {
            for (char c : string) {
                _serialFifo.push(c);
            }
        }

        void send(char c) {
            _serialFifo.push(c);
        }

        void key(char c) {
            // TODO: also handle CTRL, SHIFT, REPEAT, BREAK from sheet 14
            _pia1->setA(c);
            _pia1->assertLine(M68B21::CA1);
        }

        // Delegate functions.
        const size_t getDisplayWidth() const { return _aedRegs->getDisplayWidth(); }

        const size_t getDisplayHeight() const { return _aedRegs->getDisplayHeight(); }

        // Gets a pixel using the color map for the device
        uint32_t getPixel(int x, int y);
        const uint8_t& getRed(uint8_t index) const { return (*_redmap)[index]; }
        const uint8_t& getGreen(uint8_t index) const { return (*_grnmap)[index]; }
        const uint8_t& getBlue(uint8_t index) const { return (*_blumap)[index]; }
        const std::vector<uint8_t>& getFrameBuffer(int* sizex, int* sizey) const {
            *sizex = _aedRegs->getDisplayWidth();
            *sizey = _aedRegs->getDisplayHeight();
            return _aedRegs->getVideoMemory();
        }

    private:
        Mapper _mapper;
        M68B21 * _pia0;
        M68B21 * _pia1;
        M68B21 * _pia2;
        M68B50 * _sio0;
        M68B50 * _sio1;
        AedRegs* _aedRegs;
        uint64_t _nextHsync;
        uint64_t _nextVsync;
        uint8_t _hSync;
        uint8_t _vSync;
        Ram*    _redmap;
        Ram*    _grnmap;
        Ram*    _blumap;
        std::queue<uint8_t> _serialFifo;
};

#endif // AEDBUS_H
