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
    // Events in PriorityQueue
    enum EventType { HSYNC, VSYNC, FIELD, SERIAL };
    struct Event {
        Event(EventType type_, uint64_t time_) : type(type_), time(time_) { }
        EventType type;
        uint64_t time; // time we want the event to happen, in microseconds
    };
    struct EventCompare {
        bool operator()(const Event& lhs, const Event& rhs) const {
            return (lhs.time > rhs.time); // stored in reverse
        }
    };
    typedef std::function<void(void)> IRQ;
    typedef std::function<void(void)> NMI;
    public:
        AedBus(IRQ irq, NMI nmi);
        virtual ~AedBus();
        uint8_t read(int addr) override { return _mapper.read(addr); }
        void write(int addr, unsigned char value) override { _mapper.write(addr, value); }

        void reset() {
            _mapper.reset();
            _aedRegs->reset();
            _xon = true;
        }

        // Handles events in the priority queue based on current CPU time.
        void handleEvents(uint64_t time_us);

        // Looks for data from serial ports. Returns true if IRQ needs to happen.
        // TODO: Have this automatically invoke serial callback when requested by CPU write
        bool doSerial(uint64_t now);

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
            _pia1->set(M68B21::PortA, c);
            _pia1->assertLine(M68B21::CA1);
        }

        // Saves the current frame to a file in NetPBM format.
        void saveFrame(const std::string& path);

        // Gets the current frame from video memory, performing conversion from the
        // raw index format of the frame buffer to 24-bit color. Returned pixels are
        // in ABGR format.
        void getFrame(std::vector<uint32_t>& frame, int* width, int *height);

        // Delegate functions.
        const size_t getDisplayWidth() const { return _aedRegs->getDisplayWidth(); }

        const size_t getDisplayHeight() const { return _aedRegs->getDisplayHeight(); }

        // Gets a pixel using the color map for the device
        uint32_t getPixel(int x, int y);

    private:
        const uint8_t& getRed(uint8_t index) const { return (*_redmap)[index]; }
        const uint8_t& getGreen(uint8_t index) const { return (*_grnmap)[index]; }
        const uint8_t& getBlue(uint8_t index) const { return (*_blumap)[index]; }

        IRQ     _irq;
        NMI     _nmi;

        Mapper _mapper;
        M68B21 * _pia0;
        M68B21 * _pia1;
        M68B21 * _pia2;
        M68B50 * _sio0;
        M68B50 * _sio1;
        AedRegs* _aedRegs;
        Ram*    _redmap;
        Ram*    _grnmap;
        Ram*    _blumap;
        bool    _xon; // XON/XOFF protocol
        std::queue<uint8_t> _serialFifo;
        std::priority_queue<Event, std::vector<Event>, EventCompare> _eventQueue;
};

#endif // AEDBUS_H
