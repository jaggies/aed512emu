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
static const uint8_t SW1 = ~0x10; // negate since open is 0
static const uint8_t SW2 = ~0x7d;

// Video timing
static const uint64_t LINE_TIME = SECS2USECS(1L) / 15750;
static const uint64_t FRAME_TIME = 525*LINE_TIME/2;

// Throttle serial port if non-zero. Use if XON/XOFF is disabled on SWx above.
static const uint64_t SERIAL_HOLDOFF = 0;

AedBus::AedBus(IRQ irq, NMI nmi) : _irq(irq), _nmi(nmi), _mapper(0, CPU_MEM), _pia0(0),
        _pia1(0), _pia2(0), _sio0(0), _sio1(0), _aedRegs(0), _xon(true) {
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
    _mapper.add(_pia0 = new M68B21(pio0da, "PIA0", SW1));
    _mapper.add(_pia1 = new M68B21(pio1da, "PIA1"));
    _mapper.add(_pia2 = new M68B21(pio2da, "PIA2", SW2));
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

    // Kick off VSYNC
    _eventQueue.push(Event(VSYNC, FRAME_TIME));

    std::cerr << std::hex; // dump in hex
    std::cerr << _mapper;
}

AedBus::~AedBus() {
    std::cerr << __func__ << std::endl;
}

void AedBus::handleEvents(uint64_t now) {
    while (now > _eventQueue.top().time) {
        const Event event = _eventQueue.top();
        _eventQueue.pop();
        switch (event.type) {
            case HSYNC: {
                _eventQueue.push(Event(HSYNC, now + LINE_TIME));
                uint8_t mrd = _aedRegs->read(miscrd);
                _aedRegs->write(miscrd, (mrd & 1) ? (mrd & 0xfe) : (mrd | 0x01));
            }
            break;

            case VSYNC:
                _eventQueue.push(Event(HSYNC, now + LINE_TIME));
                _eventQueue.push(Event(VSYNC, now + FRAME_TIME));
                if (_pia1->isAssertedLine(M68B21::CB1)) {
                    _pia1->deassertLine(M68B21::CB1);
                } else {
                    _pia1->assertLine(M68B21::CB1);
                    if (_nmi != nullptr) {
                        _nmi();
                    }
                }
            break;

            case SERIAL:
                _xon = true;
            break;
        }
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
    for (size_t j = 0; j < height; j++) {
        for (size_t i = 0; i < width; i++) {
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

    for (size_t j = 0; j < height; j++) {
        for (size_t i = 0; i < width; i++) {
            uint32_t pixel = frame[(height - j - 1)*width + i];
            const uint8_t rgb[] = { uint8_t(pixel & 0xff), uint8_t((pixel >> 8) & 0xff), uint8_t(
                    (pixel >> 16) & 0xff) };
            pbm->write(pbm, rgb);
        }
    }
    pbm->close(pbm);
    free(pbm);
}

