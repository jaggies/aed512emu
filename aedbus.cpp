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
#include "aedregs.h"
#include "io.h"
#include "unknown.h"
#include "fuzz.h"

static std::vector<std::string> aed512roms = {
    "rom/512/aed_v86_c0.bin",
    "rom/512/aed_v86_c8.bin",
    "rom/512/aed_v86_d0.bin",
    "rom/512/aed_v86_d8.bin",
    "rom/512/aed_v86_e0.bin",
    "rom/512/aed_v86_e8.bin",
    "rom/512/aed_v86_f0.bin",
    "rom/512/aed_v86_f8.bin"
};

static std::vector<std::string> aed767roms = {
    "rom/767/890037-05_a325.bin",
    "rom/767/890037-04_a326.bin",
    "rom/767/890037-03_a327.bin",
    "rom/767/890037-02_a328.bin",
    "rom/767/890037-01_a329.bin"
};

#ifdef AED767
static std::vector<std::string>& roms = aed767roms;
static const size_t RAM_SIZE = 10 * 1024; // RAM
static const size_t CLUT_RED = 0x3c00;
static const size_t CLUT_GRN = 0x3d00;
static const size_t CLUT_BLU = 0x3e00;
#else
static std::vector<std::string>& roms = aed512roms;
static const size_t RAM_SIZE = 5 * 1024; // RAM
static const size_t CLUT_RED = 0x1c00;
static const size_t CLUT_GRN = 0x1d00;
static const size_t CLUT_BLU = 0x1e00;
#endif

static const size_t RAM_START = 0x00; // TODO
static const size_t CPU_MEM = 64 * 1024; // Total address space
static const size_t VIDEO_MEM = 256 * 1024; // AED 512
static const uint8_t SW1 = 0x10;
static const uint8_t SW2 = 0x7d;

AedBus::AedBus() : _mapper(0, CPU_MEM), _videoMemory(VIDEO_MEM, 0xff),
            _pia0(0), _pia1(0), _pia2(0), _sio0(0), _sio1(0) {
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
    _mapper.add(_pia0 = new M68B21(0x00, "PIA0", SW1));
    _mapper.add(_pia1 = new M68B21(0x04, "PIA1"));
    _mapper.add(_pia2 = new M68B21(0x08, "PIA2", SW2));
    _mapper.add(_sio0 = new M68B50(0x0c, "SIO0"));
    _mapper.add(_sio1 = new M68B50(0x0e, "SIO1"));
    _mapper.add(new AedRegs(0x2a, 1, "aedregs"));
    _mapper.add(new Unknown(0x10, 0x20, "DEBUG"));
    _mapper.add(new Rom(0x10000 - romBuffer.size(), romBuffer));
    _mapper.add(new Ram(RAM_START, RAM_SIZE - RAM_START));
    _mapper.add(new Ram(CLUT_RED, 0x100, "RED"));
    _mapper.add(new Ram(CLUT_GRN, 0x100, "GRN"));
    _mapper.add(new Ram(CLUT_BLU, 0x100, "BLU"));
    _mapper.add(new Unknown(0, CPU_MEM));

    std::cerr << std::hex; // dump in hex
    std::cerr << _mapper;
}

AedBus::~AedBus() {
}

