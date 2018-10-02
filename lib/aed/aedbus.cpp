/*
 * aedbus.cpp
 *
 *  Created on: Sep 8, 2018
 *      Author: jmiller
 */

#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <vector>
#include <string>
#include "netpbm.h"
#include "aedbus.h"
#include "ram.h"
#include "generic.h"
#include "io.h"

static bool debug = false;

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
static const int8_t SW2 = 0xed; // Comm 1-8: [Xon, ForceRTS, AuxBaud[3..5], HostBaud[6..8]]

// Video timing
static const size_t VTOTAL = 525; // 262.5 lines per field
static const size_t VBLANK_DURATION = 20; // VBLANK duration in scanlines (per field)
static const size_t HBLANK_DURATION_US = 12; // Actually 10.7us
static const uint64_t LINE_TIME_US = SECS2USECS(1L) / 15750;

// Throttle serial port if non-zero. Use if XON/XOFF is disabled on SWx above.
static const uint64_t SERIAL_HOLDOFF = 0;

AedBus::AedBus(IRQ irq, NMI nmi) : _irq(irq), _nmi(nmi), _mapper(0, CPU_MEM),
        _pia0(nullptr), _pia1(nullptr), _pia2(nullptr),
        _sio0(nullptr), _sio1(nullptr), _aedRegs(nullptr), _xon(true), _scanline(0) {

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

    // Add peripherals. Earlier peripherals are favored when addresses overlap.
    _mapper.add(_pia0 = new M68B21(pio0da, "PIA0", [this](int) { _irq(); }, [this](int) {_irq(); } ));
    _mapper.add(_pia1 = new M68B21(pio1da, "PIA1", [this](int) { _irq(); }, [this](int) {_nmi(); } ));
    _mapper.add(_pia2 = new M68B21(pio2da, "PIA2", [this](int) { _irq(); }, [this](int) {_irq(); } ));
    _mapper.add(_sio0 = new M68B50(sio0st, "SIO0"));
    _mapper.add(_sio1 = new M68B50(sio1st, "SIO1"));
#if !defined(AED767) && !defined(AED1024)
    _mapper.add(new Generic(0xe9, 1,
            [this](int offset) { return 0x01; },
            [this](int offset, uint8_t value) {
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
//    _mapper.add(new Generic(0x3e, 2,
//            [this](int offset) { return offset ? 0x02 : 0xff; },
//            [this](int offset, uint8_t value) {
//                if (debug) std::cerr << "write " << (int)(offset + 0x3e) << ":" << (int) value << std::endl;
//            },
//            "hack_lines"));
    _mapper.add(new Ram(0x8000, 0x300, "hack_0x8000"));
    _mapper.add(new RamDebug(ACAIK_BASE, SRAM_SIZE, "ACAIK"));
#endif
    _mapper.add(_aedRegs = new AedRegs(0x00, 0x30, "aedregs"));
    _mapper.add(new Rom(0x10000 - romBuffer.size(), romBuffer));
    _mapper.add(new Ram(LED_BASE, SRAM_SIZE, "LED"));
    _mapper.add(new Ram(RAM_START, RAM_SIZE - RAM_START));
    _mapper.add(_redmap = new Ram(CLUT_RED, 0x100, "RED"));
    _mapper.add(_grnmap = new Ram(CLUT_GRN, 0x100, "GRN"));
    _mapper.add(_blumap = new Ram(CLUT_BLU, 0x100, "BLU"));
    _mapper.add(new RamDebug(0, CPU_MEM, "unmapped"));

    // Kick off HBLANK
    _eventQueue.push(Event(HBLANK, 0));

    std::cerr << std::hex; // dump in hex
    std::cerr << _mapper;

    // Set up DIP switch settings
    _pia0->set(M68B21::PortA, ~SW1);
    _pia2->set(M68B21::PortA, ~SW2);
}

AedBus::~AedBus() {
    std::cerr << __func__ << std::endl;
}

void
AedBus::reset() {
   _mapper.reset(); // This resets all peripherals
   _pia0->set(M68B21::PortA, ~SW1); // update DIP switch settings
   _pia2->set(M68B21::PortA, ~SW2);
   _xon = true;
}

// PIA1 Signal pins
const int VERTBLANK_SIGNAL = M68B21::CB1;
const int FIELD_SIGNAL = M68B21::PB6;
//const int INT2_5_SIGNAL = M68B21::PB7; // PIA1 Joystick comparator

void AedBus::handleEvents(uint64_t now) {
    // TODO: when swapping this with an if statement, the video timing is correct, but
    // the device buffer overflows because it never emits XOFF.
    if (now > _eventQueue.top().time) {
        const Event& event = _eventQueue.top();
        switch (event.type) {
            case HSYNC: {
                // Assert HBLANK
                _aedRegs->write(miscrd, _aedRegs->read(miscrd) | 0x01);
            }
            break;
            case HBLANK: {
                // Deassert HBLANK
                _aedRegs->write(miscrd, _aedRegs->read(miscrd) & 0xfe);

                // Assert VBLANK for first 20 scan lines after sync
                int fieldline = _scanline > VTOTAL / 2 ? _scanline - VTOTAL / 2 : _scanline;
                if (fieldline > (VTOTAL / 2 - VBLANK_DURATION)) {
                    _pia1->set(M68B21::IrqStatusB, VERTBLANK_SIGNAL);
                } else {
                    _pia1->reset(M68B21::IrqStatusB, VERTBLANK_SIGNAL);
                }

                // Assert FIELD_SIGNAL for second field
                if (_scanline < VTOTAL / 2) {
                    _pia1->set(M68B21::PortB, FIELD_SIGNAL);
                } else {
                    _pia1->reset(M68B21::PortB, FIELD_SIGNAL);
                }

                _scanline++;
                if (_scanline >= VTOTAL) _scanline = 0;
                _eventQueue.push(Event(HSYNC, now + LINE_TIME_US));
                _eventQueue.push(Event(HBLANK, now + LINE_TIME_US - HBLANK_DURATION_US));
            }
            break;

            case SERIAL:
                _xon = true;
            break;
        }
        _eventQueue.pop();
    }
    // TODO: make this event-based
    if (_irq != nullptr && doSerial(now)) {
        _irq();
    }
}

// Handles serial ports. Returns true if IRQ was generated
bool
AedBus::doSerial(uint64_t now) {
   uint8_t byte;
   if (_sio0->transmit(&byte)) {
       std::cout << "SIO0: " << (int) byte << std::endl;
   }

   if (_sio1->transmit(&byte)) {
       std::cout << "SIO1: " << (int) byte << std::endl;
       if (byte == 19) { // XOFF
           _xon = false;
       } else if (byte == 17) { // XON
           _xon = true;
       }
   }

   if (_xon && !_serialFifo.empty() && _sio1->receive(_serialFifo.front())) {
       _xon = false;
       _eventQueue.push(Event(SERIAL, now + SERIAL_HOLDOFF));
       _serialFifo.pop();
   }
   return _sio1->irqAsserted();
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
    int width = *w = _aedRegs->getDisplayWidth();
    int height = *h = _aedRegs->getDisplayHeight();
    const std::vector<uint8_t>& raw = _aedRegs->getVideoMemory();
    if (frame.size() != raw.size()) {
        frame.resize(raw.size());
    }
    // Convert index to color using LUT. TODO: use OpenGL to do the final conversion
    const uint8_t* red = &getRed(0);
    const uint8_t* grn = &getGreen(0);
    const uint8_t* blu = &getBlue(0);

    const int scrollx = _aedRegs->getScrollX();
    const int scrolly = _aedRegs->getScrollY();
    for (int j = 0; j < height; j++) {
        for (int i = 0; i < width; i++) {
            const size_t x = (i + scrollx) % width;
            const size_t y = (j + scrolly) % height;
            const uint8_t lut = raw[y * width + x];
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

