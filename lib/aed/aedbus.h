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
    enum EventType { FIELD = 0, VBLANK_P, VBLANK_N, HBLANK_P, HBLANK_N,
        SERIAL, JOYSTICK_SET, JOYSTICK_RESET };
    struct Event {
        Event(EventType type_, uint64_t time_us) : type(type_), time(time_us) { }
        EventType type;
        uint64_t time; // time we want the event to happen, in microseconds
    };
    struct EventCompare {
        bool operator()(const Event& lhs, const Event& rhs) const {
            return (lhs.time > rhs.time); // stored in reverse
        }
    };
    typedef std::function<void(void)> Redraw;

    public:
        AedBus(Peripheral::IRQ irq, Peripheral::IRQ nmi, Redraw redraw = nullptr);
        virtual ~AedBus();
        uint8_t read(int addr) override { return _mapper.read(addr); }
        void write(int addr, unsigned char value) override { _mapper.write(addr, value); }

        void reset();

        // Handles events in the priority queue based on current CPU time.
        void handleEvents(uint64_t time_us);

        // Gets next event from queue
        const Event& getNextEvent() const { return _eventQueue.top(); }

        // Handles input FIFO for serial ports
        void doSerial(uint64_t now);

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
            _pia1->reset(M68B21::InputA, 0xff);
            _pia1->set(M68B21::InputA, c);
            _pia1->set(M68B21::ControlA, M68B21::CA1);
            _pia1->reset(M68B21::ControlA, M68B21::CA1);
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

        void setCpuTime(uint64_t t) { _cpuTime = t; }
        void setJoystick(uint16_t x, uint16_t y) { _joyX = x; _joyY = y; }

    private:
        const uint8_t& getRed(uint8_t index) const { return (*_redmap)[index]; }
        const uint8_t& getGreen(uint8_t index) const { return (*_grnmap)[index]; }
        const uint8_t& getBlue(uint8_t index) const { return (*_blumap)[index]; }

        void handlePIA0(M68B21::Port port, uint8_t oldData, uint8_t newData);
        void handlePIA1(M68B21::Port port, uint8_t oldData, uint8_t newData);
        void handlePIA2(M68B21::Port port, uint8_t oldData, uint8_t newData);
        bool handleSIO0(uint8_t byte);
        bool handleSIO1(uint8_t byte);
        void field();
        void vblank(bool set);
        void hblank(bool set);

        Peripheral::IRQ _irq;
        Peripheral::IRQ _nmi;

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
        bool    _erase = false; // hardware is in erase cycle when true; erase each scanline during scan
        bool    _xon = true; // XON/XOFF protocol
        int     _scanline = 0; // the current scanline
        uint64_t    _cpuTime = 0; // current CPU time. TODO: have this be the source of truth
        uint16_t    _joyX = 0; // X joystick input, range [0, 511]
        uint16_t    _joyY = 0; // Y joystick input, range [0, 511]
        uint64_t    _joyDelay = 0; // delay for joyX and joyY, depending on last selection cycle
        std::queue<uint8_t> _serialFifo;
        std::priority_queue<Event, std::vector<Event>, EventCompare> _eventQueue;
        Redraw  _redraw;
        bool    _mwe; // memory write enable? Used to debug erase hardwares
};

#endif // AEDBUS_H
