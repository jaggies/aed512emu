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

        // Handle video-related timing and return 'true' if IRQ needs to happen
        // TODO: Handle this better. Maybe use SIGUSR interrupt.
        bool doVideo();

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

        // Delegate functions.
        const size_t getDisplayWidth() const { return _aedRegs->getDisplayWidth(); }

        const size_t getDisplayHeight() const { return _aedRegs->getDisplayHeight(); }

        // Gets a pixel using the color map for the device
        uint32_t getPixel(int x, int y);

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
        std::queue<uint8_t> _serialFifo;
};

#endif // AEDBUS_H
