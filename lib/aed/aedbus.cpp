/*
 * aedbus.cpp
 *
 *  Created on: Sep 8, 2018
 *      Author: jmiller
 */

#include <iostream>
#include <fstream>
#include <cmath>
#include <sys/stat.h>
#include <vector>
#include <string>
#include "netpbm.h"
#include "aedbus.h"
#include "ram.h"
#include "generic.h"
#include "io.h"
#include "coreutil.h"

static bool debug = false;
static bool debug_timing = false;

// PIA1 Signal pins
const int VBLANK_SIGNAL = M68B21::CB1;
const int PIXCNTOFLO_SIGNAL = M68B21::CA2;
const int ERASE_SIGNAL = M68B21::CRB3; // CB2 output bit
//const int ERASE_ENABLE = M68B21::CRB4; // CB2 is not in handshake mode
const int ERASE_OUTPUT = M68B21::CRB5; // CB2 is enabled as output
const int FIELD_SIGNAL = M68B21::PB6;
const int ADCH0 = M68B21::PB0;
const int ADCH1 = M68B21::PB1;
const int REFS = M68B21::PB2;
const int JSTK = M68B21::PB3;
const int INT2_5_SIGNAL = M68B21::PB7; // PIA1 Joystick comparator

// PIA2 - These seem to be involved in SuperRoam
//const int X9_SIGNAL = M68B21::CA2;
//const int Y9_SIGNAL = M68B21::CB2;

#if defined(AED767) || defined(AED1024)
#define SRAM_SIZE 2048
#define CLUT_BASE 0x3c00
static std::vector<std::string> roms = {
    "rom/767/890037-05_a325.bin",
    "rom/767/890037-04_a326.bin",
    "rom/767/890037-03_a327.bin",
    "rom/767/890037-02_a328.bin",
    "rom/767/890037-01_a329.bin"
};
#else
#define SRAM_SIZE 1024
#define CLUT_BASE 0x1c00
static std::vector<std::string> roms = {
    "rom/512/aed_v86_c0.bin",
    "rom/512/aed_v86_c8.bin",
    "rom/512/aed_v86_d0.bin",
    "rom/512/aed_v86_d8.bin",
    "rom/512/aed_v86_e0.bin",
    "rom/512/aed_v86_e8.bin",
    "rom/512/aed_v86_f0.bin",
    "rom/512/aed_v86_f8.bin"
};
#endif
static const size_t RAM_SIZE = 5 * SRAM_SIZE; // RAM
#if defined(AED767) || defined(AED1024)
static const size_t ACAIK_BASE = RAM_SIZE; // AED 767/1024 only
#endif
static const size_t LED_BASE = 6 * SRAM_SIZE;
static const size_t RAM_START = 0x00; // TODO
static const size_t CLUT_RED = CLUT_BASE;
static const size_t CLUT_GRN = CLUT_BASE + 256;
static const size_t CLUT_BLU = CLUT_BASE + 512;
static const size_t CPU_MEM = 64 * 1024; // Total address space

// DIP Switches. Open is 1, closed is 0
static const int8_t SW1 = 0x10; // Option 1-8:[Fduplex, Erase, Rubout, Tk4014, ParReset, A2, A1, A0]
static const int8_t SW2 = 0x7d; // Comm 1-8: [Xoff, ForceRTS, AuxBaud[3..5], HostBaud[6..8]]

// Video timing
static const int VTOTAL = 525;
static const int VISIBLE_LINES = 483;
static const float FIELD_RATE = 59.97; // yay, NTSC
static const float FIELD_TIME_US = (float) SECS2USECS(1) / FIELD_RATE;
static const float LINE_TIME_US = FIELD_TIME_US / ((float) VTOTAL / 2.0f); // half since fields
static const float VBLANK_P_US = FIELD_TIME_US * VISIBLE_LINES / VTOTAL;
static const float VBLANK_N_US =  FIELD_TIME_US * (VTOTAL - VISIBLE_LINES) / VTOTAL;
static const float HBLANK_P_US = 22.92f; // 22.92 measured from device
static const float HBLANK_N_US = LINE_TIME_US - HBLANK_P_US;
static const float FIELD_DLY_US = 387; // 387us (delay from VBLANK_N) measured
static const float HBLANK_DLY_US = 15.8; // 15.8us (delay from VBLANK_N) measured

// Throttle serial port if non-zero. Use if XON/XOFF is disabled on SWx above.
static const uint64_t SERIAL_HOLDOFF = 0;

void AedBus::handlePIA0(M68B21::Port port, uint8_t oldData, uint8_t newData) {
    uint8_t changed = oldData ^ newData;
    switch (port) {
            case M68B21::OutputB:
                if (changed & 1) {
                    _xs8 = newData & 1;
                }
                if (changed & (0x1c)) {
                    _xzoom = (newData & 0x1c) >> 2;
                    // std::cerr << "pia_zoomx: " << zoomX << std::endl;
                }
            break;
            case M68B21::ControlA:
                if (rising(oldData, newData, M68B21::CRA5)) {
                    if (newData & M68B21::CRA4) {
                        if (debug) std::cerr << "PIA0: CA2 output enabled" << std::endl;
                    } else {
                        if (debug) std::cerr << "PIA0: CA2 output E-clock enabled" << std::endl;
                    }
                }
                if (changed & M68B21::CRA3) {
                    std::cerr << "PIA0: CA2:" << bool(newData & M68B21::CRB3) << std::endl;
                }
            break;
            case M68B21::ControlB:
                if (rising(oldData, newData, M68B21::CRB5)) {
                    if (newData & M68B21::CRB4) {
                        if (debug) std::cerr << "PIA0: CB2 output enabled" << std::endl;
                    } else {
                        if (debug) std::cerr << "PIA0: CB2 output E-clock enabled" << std::endl;
                    }
                }
                if (changed & M68B21::CRB3) {
                    std::cerr << "PIA0: CB2:" << bool(newData & M68B21::CRB3) << std::endl;
                }
            break;
            default: break; // get rid of warning
    }
}

void AedBus::handlePIA1(M68B21::Port port, uint8_t oldData, uint8_t newData) {
    const uint8_t changed = oldData ^ newData;
    switch (port) {
        case M68B21::ControlA:
            if (debug) {
                std::cerr << _pia1->name() << " CrA changed: " << (int) (newData) << std::endl;
            }
        break;
        case M68B21::ControlB:
            if (debug) {
                std::cerr << _pia1->name() << " CrB changed: " << (int) (newData) << std::endl;
            }
            if (newData & ERASE_OUTPUT) { // CB2 configured as output
                if (changed & ERASE_SIGNAL) {
                    _erase = newData & ERASE_SIGNAL;
                    std::cerr << "ERASE:" << (int) _erase << std::endl;
                }
            }
        break;
        case M68B21::OutputB:
            if (changed & 0x10) {
                // Memory Write Enable (enables 6502 writing to video memory?)

                // It appears that the system selects the color through non-incrementing XY
                // and then expects the hardware to clear one 16-bit video memory word per clock.
                // how to tie this in...

                _mwe = bool(newData & 0x10);
                std::cerr << "MWE:" << _mwe << " at " << std::dec << _cpuTime << std::endl;
            }
            if (changed & 0x0f) {
                // Joystick processing
                switch (newData & 0x0f) {
                    case 0x00: // disconnected
                    break;
                    case (REFS | ADCH1): // 4V on inverting amp => INT2.5V low
                        _pia1->set(M68B21::InputB, INT2_5_SIGNAL);
                        addEvent(Event(JOYSTICK_RESET, _cpuTime + _joyDelay));
                    break;
                    case (REFS | ADCH1 | ADCH0): // GND on inverting amp => INT2.5V high
                        addEvent(Event(JOYSTICK_SET, _cpuTime));
                    break;
                    case JSTK: // Y joystick tracking - charge Y
                        _joyDelay = 5 * (512 - _joyY); // Y is inverted by hw design.
                    break;
                    case (JSTK | ADCH0): // X joystick tracking - charge X
                        _joyDelay = 5 * _joyX;
                    break;
                }
            }
        break;

        default: // fix warning
        break;
    }
}

void AedBus::handlePIA2(M68B21::Port port, uint8_t oldData, uint8_t newData) {
    uint8_t changed = oldData ^ newData;
    switch (port) {
        case M68B21::OutputB:
            _ys8 = newData & 1;
            if (debug) std::cerr << "PIA2, PortB: " << newData << std::endl;
        break;
        case M68B21::ControlA:
            if (rising(oldData, newData, M68B21::CRA5)) {
                if (debug) std::cerr << "PIA2: CA2 output enabled" << std::endl;
            }
            if (changed & M68B21::CRA3) {
                if (debug) std::cerr << "PIA2: CA2:" << bool(newData & M68B21::CRA3) << std::endl;
            }
        break;
        case M68B21::ControlB:
            if (rising(oldData, newData, M68B21::CRB5)) {
                if (debug) std::cerr << "PIA2: CB2 output enabled" << std::endl;
            }
            if (changed & M68B21::CRB3) {
                if (debug) std::cerr << "PIA2: CB2:" << bool(newData & M68B21::CRB3) << std::endl;
            }
        break;
        default: break; // get rid of warning
    }
}

bool AedBus::handleSIO0(uint8_t byte) {
    std::cout << "SIO0: " << (int) byte << std::endl;
    return true; // handled
}

bool AedBus::handleSIO1(uint8_t byte) {
    switch(byte) {
        //case 10: std::cout << (char) 13; break; // LF
        case 13: std::cout << (char) 10; break; // CR
        case 17: _xon = true; break; // XON
        case 19: _xon = false; break; // XOFF
        default:
            std::cout << byte << std::flush;
            break;
    }

    return true; // handled
}

void AedBus::field() {
    bool set = _pia1->isSet(M68B21::InputB, FIELD_SIGNAL);
    if (debug_timing) std::cerr << __func__ << ":" << set << " at " << _cpuTime << std::endl;
    if (set) {
        _pia1->reset(M68B21::InputB, FIELD_SIGNAL);
    } else {
        _pia1->set(M68B21::InputB, FIELD_SIGNAL);
    }
}

void AedBus::vblank(bool set) {
    if (debug_timing) std::cerr << __func__ << ":" << set << " at " << _cpuTime << std::endl;
    if (set) {
        _pia1->set(M68B21::ControlB, VBLANK_SIGNAL);
    } else {
        _pia1->reset(M68B21::ControlB, VBLANK_SIGNAL);
    }
}

void AedBus::hblank(bool set) {
    if (debug_timing) std::cerr << __func__ << ":" << set << " at " << _cpuTime << std::endl;
    if (set) {
        _aedRegs->write(miscrd, _aedRegs->read(miscrd) | 0x01);
    } else {
        _aedRegs->write(miscrd, _aedRegs->read(miscrd) & 0xfe);
    }
}

AedBus::AedBus(Peripheral::IRQ irq, Peripheral::IRQ nmi, Redraw redraw)
        : _irq(irq), _nmi(nmi), _mapper(0, CPU_MEM),
        _pia0(nullptr), _pia1(nullptr), _pia2(nullptr),
        _sio0(nullptr), _sio1(nullptr), _aedRegs(nullptr), _redraw(redraw), _mwe(false) {

    // Open all ROM files and copy to ROM location in romBuffer
    std::vector<uint8_t> romBuffer;
    size_t offset = 0;
    std::vector<std::string> files;
    if (char* path = getenv("ROMFILE")) {
        std::cerr << "Reading ROM from " << path << std::endl;
        files.push_back(path);
    } else {
        files = roms;
    }
    for(const std::string& romFileName : files) {
        std::ifstream file(romFileName, std::ios::binary | std::ios::ate);
        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);
        romBuffer.resize(romBuffer.size() + size);
        if (file.read((char*)&romBuffer[offset], size)) {
            offset += size;
        }
    }
    // Add peripherals. Lower address peripherals are favored when addresses overlap.
    _mapper.add(_pia0 = new M68B21(pio0da, "PIA0", [this]() { _irq(); }, [this]() {_irq(); },
            [this](M68B21::Port port, uint8_t old, uint8_t new_) { handlePIA0(port, old, new_); }));
    _mapper.add(_pia1 = new M68B21(pio1da, "PIA1", [this]() { _irq(); }, [this]() { _nmi(); },
            [this](M68B21::Port port, uint8_t old, uint8_t new_) { handlePIA1(port, old, new_); },
            [](bool isSet) { std::cerr << "CA2:" << isSet << std::endl; },
            [](bool isSet) { std::cerr << "CB2(erase):" << isSet << std::endl; }
            ));
    _mapper.add(_pia2 = new M68B21(pio2da, "PIA2", [this]() { _irq(); }, [this]() {_irq(); },
            [this](M68B21::Port port, uint8_t old, uint8_t new_) { handlePIA2(port, old, new_); }));
    _mapper.add(_sio0 = new M68B50(sio0st, "SIO0", [this]() { _irq(); },
            [this](uint8_t byte) { return handleSIO0(byte); }));
    _mapper.add(_sio1 = new M68B50(sio1st, "SIO1", [this]() { _irq(); },
            [this](uint8_t byte) { return handleSIO1(byte); }));
#if !defined(AED767) && !defined(AED1024)
    _mapper.add(new Generic(0xe9, 1,
            [](int offset) { return 0x01; },
            [](int offset, uint8_t value) {
                if (debug) std::cerr << "write 0xe9:" << (int) value << std::endl;
            },
            "hack_0xe9"));
#else
    _mapper.add(new Generic(0xe5, 1,
            [this](int offset) { return 0xff; },
            [this](int offset, uint8_t value) {
                if (debug) std::cerr << "write 0xe5:" << (int) value << std::endl;
            },
            "hack_0xe5"));
    _mapper.add(new Ram(0x8000, 0x300, "hack_0x8000"));
    _mapper.add(new RamDebug(ACAIK_BASE, SRAM_SIZE, "ACAIK"));
#endif
    _mapper.add(_aedRegs = new AedRegs(0x00, 0x30, "aedregs",
            [this](bool set) {
                if (set) {
                    _pia0->set(M68B21::ControlA, PIXCNTOFLO_SIGNAL);
                } else {
                    _pia0->reset(M68B21::ControlA, PIXCNTOFLO_SIGNAL);
                }
    }));
    _mapper.add(new Rom(0x10000 - romBuffer.size(), romBuffer));
    _mapper.add(new Ram(LED_BASE, SRAM_SIZE, "LED"));
    _mapper.add(new Ram(RAM_START, RAM_SIZE - RAM_START));
    _mapper.add(_redmap = new Ram(CLUT_RED, 0x100, "RED"));
    _mapper.add(_grnmap = new Ram(CLUT_GRN, 0x100, "GRN"));
    _mapper.add(_blumap = new Ram(CLUT_BLU, 0x100, "BLU"));
    _mapper.add(new RamDebug(0, CPU_MEM, "unmapped"));

    // Kick off VBLANK_N
    vblank(1);
    hblank(1);
    addEvent(Event(VBLANK_N, 0)); // Allow CPU to run for a bit before sending IRQs

    std::cerr << std::hex; // dump in hex
    std::cerr << _mapper;

    // Set up DIP switch settings
    _pia0->set(M68B21::InputA, ~SW1);
    _pia2->set(M68B21::InputA, ~SW2);
}

AedBus::~AedBus() {
    std::cerr << __func__ << std::endl;
}

void
AedBus::reset() {
   _mapper.reset(); // This resets all peripherals
   _pia0->set(M68B21::InputA, ~SW1); // update DIP switch settings
   _pia2->set(M68B21::InputA, ~SW2);
   _scanline = 0;
   _cpuTime = 0; // TODO: reset external clocl that controls this one
   _joyDelay = _joyX = _joyY = 0;
   _erase = false;
   _xs8 = 0;
   _ys8 = 0;
   _xon = true;
}

void AedBus::handleEvents(uint64_t now) {
    Event event;
    while (getNextEvent(now, &event)) {
        switch (event.type) {
            case HBLANK_P:
                hblank(1);
            break;

            case HBLANK_N:
                hblank(0);
                _scanline++;
            break;

            case VBLANK_P:
                vblank(1);
                if (_redraw) {
                    _redraw();
                }
            break;

            case VBLANK_N: {
                vblank(0);
                _scanline = 0;
                addEvent(Event(VBLANK_P, event.time + VBLANK_N_US));
                addEvent(Event(FIELD, event.time + FIELD_DLY_US));
                // Add all horizontal retraces
                float t = HBLANK_DLY_US;
                int lines = 0;
                while (t < FIELD_TIME_US) {
                    addEvent(Event(HBLANK_N, event.time + roundl(t)));
                    addEvent(Event(HBLANK_P, event.time + roundl(t + HBLANK_N_US) ));
                    t += HBLANK_N_US + HBLANK_P_US;
                    lines++;
                    assert(lines <= ceilf(VTOTAL/2.0f)); // for interlaced signal
                }
                // Rinse and repeat...
                addEvent(Event(VBLANK_N, event.time + VBLANK_N_US + VBLANK_P_US));
            }
            break;

            case FIELD:
                field();
            break;

            case SERIAL:
                _xon = true;
            break;

            case KEY_DOWN:
                _pia1->reset(M68B21::InputA, 0xff);
                _pia1->set(M68B21::InputA, event.arg0);
                _pia1->set(M68B21::ControlA, M68B21::CA1);
                _pia1->reset(M68B21::ControlA, M68B21::CA1);
            break;

            case JOYSTICK_SET:
                _pia1->set(M68B21::InputB, INT2_5_SIGNAL);
            break;

            case JOYSTICK_RESET:
                _pia1->reset(M68B21::InputB, INT2_5_SIGNAL);
            break;

            case NONE: // fix warning
            break;
        }
    }
    doSerial(now); // TODO: make this entirely event-based
}

// Checks for data available in serial port FIFO and sends to device.
void
AedBus::doSerial(uint64_t now) {
   std::lock_guard<std::mutex> lock(_mutex);
   if (_xon && !_serialFifo.empty() && _sio1->receive(_serialFifo.front())) {
       if (SERIAL_HOLDOFF > 0) {
           _xon = false;
           addEvent(Event(SERIAL, now + SERIAL_HOLDOFF));
       }
       _serialFifo.pop();
   }
}

uint32_t
AedBus::getPixel(int x, int y)
{
    uint8_t idx = _aedRegs->pixel(x, y);
    uint8_t red = _mapper.read((int) CLUT_RED + idx);
    uint8_t grn = _mapper.read((int) CLUT_GRN + idx);
    uint8_t blu = _mapper.read((int) CLUT_BLU + idx);
    return 0xff000000 | (blu << 16) | (grn << 8) | red;
}

void
AedBus::getFrame(std::vector<uint32_t>& frame, int* w, int *h) {
    size_t width = *w = _aedRegs->getDisplayWidth();
    size_t height = *h = _aedRegs->getDisplayHeight();
    const std::vector<uint8_t>& raw = _aedRegs->getVideoMemory();
    if (frame.size() != raw.size()) {
        frame.resize(raw.size());
    }
    // Convert index to color using LUT. TODO: use OpenGL to do the final conversion
    const uint8_t* red = &getRed(0);
    const uint8_t* grn = &getGreen(0);
    const uint8_t* blu = &getBlue(0);

    const size_t scrollx = ((_xs8 ? 0x100 : 0x000) | _aedRegs->getScrollX());
    const size_t scrolly = ((_ys8 ? 0x100 : 0x000) | _aedRegs->getScrollY());
    const int zoomX = _aedRegs->getZoomX();
    const int zoomY = _aedRegs->getZoomY();
    for (size_t j = 0; j < height; j++) {
        for (size_t i = 0; i < width; i++) {
            const size_t x = (i/zoomX + scrollx) % width;
            const size_t y = (j/zoomY - scrolly) % height;
            const uint8_t lut = raw[((height - 1) - y) * width + x];
            frame[j * width + i] = 0xff000000 | (blu[lut] << 16) | (grn[lut] << 8) | (red[lut]);
        }
    }
}

void
AedBus::saveFrame(const std::string& path) {
    NetPBM* pbm = createNetPBM();
    assert(path.size() > 0);
    int depth = 255;
    int width = 0;
    int height = 0;
    std::vector<uint32_t> frame;
    getFrame(frame, &width, &height);

    if (path.size() == 0 || !pbm->open(pbm, path.c_str(), &width, &height, &depth, NETPBM_WRITE)) {
        std::cerr << "Can't write image " << path << std::endl;
        return;
    }

    for (int j = 0; j < height; j++) {
        for (int i = 0; i < width; i++) {
            uint32_t pixel = frame[(height - j - 1)*width + i];
            const uint8_t rgb[] = { uint8_t(pixel & 0xff), uint8_t((pixel >> 8) & 0xff), uint8_t(
                    (pixel >> 16) & 0xff) };
            pbm->write(pbm, rgb);
        }
    }
    pbm->close(pbm);
    free(pbm);
}

