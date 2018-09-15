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

class AedBus : public BUS {
    #define SECS2USECS(a) ((a)*1000000)
    public:
        AedBus();
        virtual ~AedBus();
        uint8_t read(int addr) override { return _mapper.read(addr); }
        void write(int addr, unsigned char value) override { _mapper.write(addr, value); }

        // TODO: Use SIGALARM instead of calculating this every time!
        bool doVideo() {
            struct timeval tp = { 0 };
            gettimeofday(&tp, NULL);
            uint64_t now = tp.tv_sec * SECS2USECS(tp.tv_sec) + tp.tv_usec;
            if (now > _nextVideoSync) {
                _pia1->assertLine(M68B21::CB1);
                _nextVideoSync = now + SECS2USECS(60) / 60; // 60Hz
                return true;
            } else {
                _pia1->deassertLine(M68B21::CB1);
                return false;
            }
        }

        // Handles serial ports. Returns true if IRQ was generated
        bool doSerial() {
            uint8_t byte;
            if (_sio0->transmit(&byte)) {
                std::cout << "SIO0: " << (int) byte << std::endl;
            }

            if (_sio1->transmit(&byte)) {
                std::cout << "SIO1: " << (int) byte << std::endl;
            }

            if (!_serialFifo.empty() && _sio0->receive(_serialFifo.front())) {
                _serialFifo.pop();
            }
            return _sio0->irqAsserted() || _sio1->irqAsserted();
        }

        void send(const std::string& string) {
            for (char c : string) {
                _serialFifo.push(c);
            }
        }

    private:
        Mapper _mapper;
        std::vector<uint8_t> _videoMemory;
        M68B21 * _pia0;
        M68B21 * _pia1;
        M68B21 * _pia2;
        M68B50 * _sio0;
        M68B50 * _sio1;
        uint64_t _nextVideoSync;
        std::queue<uint8_t> _serialFifo;
};

#endif // AEDBUS_H
