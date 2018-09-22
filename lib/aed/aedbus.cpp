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
#include "aedbus.h"
#include "ram.h"
#include "generic.h"
#include "io.h"

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
static const uint64_t LINE_TIME = SECS2USECS(1L) / 40000;
static const uint64_t FRAME_TIME = 525*LINE_TIME;

AedBus::AedBus() : _mapper(0, CPU_MEM), _pia0(0), _pia1(0), _pia2(0),
        _sio0(0), _sio1(0), _aedRegs(0), _nextHsync(0), _nextVsync(0), _hSync(0), _vSync(0) {
    // Open all ROM files and copy to ROM location in romBuffer
    std::vector<uint8_t> romBuffer;
    size_t offset = 0;
    for(const std::string& romFileName : roms) {
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
            [this](int offset, uint8_t value) { std::cerr << "write 0xe9:" << (int) value << std::endl;  },
            "hack_0xe9"));

#else
    _mapper.add(new Generic(0xe5, 1,
            [this](int offset) { return 0xff; },
            [this](int offset, uint8_t value) { std::cerr << "write 0xe5:" << (int) value << std::endl;  },
            "hack_0xe5"));
    _mapper.add(new Generic(0x3e, 2,
                [this](int offset) { return offset ? 0x02 : 0xff; },
                [this](int offset, uint8_t value) { std::cerr << "write " << (int)(offset + 0x3e) << ":" << (int) value << std::endl;  },
                "hack_lines"));
    _mapper.add(new Ram(0x8000, 0x300, "FOO"));
    _mapper.add(new RamDebug(ACAIK_BASE, SRAM_SIZE, "ACAIK"));
#endif
    _mapper.add(new Generic(miscrd, 1,
                [this](int offset) { return this->_hSync; },
                [this](int offset, uint8_t value) { std::cerr << "write 0x2a:" << (int) value << std::endl; },
                "hack_miscrd"));
    _mapper.add(_aedRegs = new AedRegs(0x00, 0x30, "aedregs"));
    _mapper.add(new Rom(0x10000 - romBuffer.size(), romBuffer));
    _mapper.add(new Ram(LED_BASE, SRAM_SIZE, "LED"));
    _mapper.add(new Ram(RAM_START, RAM_SIZE - RAM_START));
    _mapper.add(_redmap = new Ram(CLUT_RED, 0x100, "RED"));
    _mapper.add(_grnmap = new Ram(CLUT_GRN, 0x100, "GRN"));
    _mapper.add(_blumap = new Ram(CLUT_BLU, 0x100, "BLU"));
    _mapper.add(new RamDebug(0, CPU_MEM, "unmapped"));

    std::cerr << std::hex; // dump in hex
    std::cerr << _mapper;
}

AedBus::~AedBus() {
}

bool
AedBus::doVideo(uint64_t time_us) {
   bool doIrq = false;
   struct timeval tp = { 0 };
   gettimeofday(&tp, NULL);
   if (time_us > _nextVsync) {
       _vSync = 1;
       _nextVsync = time_us + FRAME_TIME; // 60Hz
       doIrq = true;
   } else if (time_us > _nextVsync - LINE_TIME) {
       _vSync = 0;
   }
   if (time_us > _nextHsync) {
       _hSync = !_hSync;
       _nextHsync = time_us + LINE_TIME/800; // Hack - toggle twice per 15kHz sync
   }
   _vSync ? _pia1->assertLine(M68B21::CB1) : _pia1->deassertLine(M68B21::CB1);
   return doIrq;
}

// Handles serial ports. Returns true if IRQ was generated
bool
AedBus::doSerial() {
   uint8_t byte;
   if (_sio0->transmit(&byte)) {
       std::cout << "SIO0: " << (int) byte << std::endl;
   }

   if (_sio1->transmit(&byte)) {
       std::cout << "SIO1: " << (int) byte << std::endl;
   }

   if (!_serialFifo.empty() && _sio1->receive(_serialFifo.front())) {
       _serialFifo.pop();
   }
   return _sio0->irqAsserted() || _sio1->irqAsserted();
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
