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
#include "io.h"
#include "ram.h"
#include "rom.h"
#include "68B21.h"
#include "68B50.h"
#include "unknown.h"
#include "fuzz.h"

static const size_t RAM_START = 0x00; // TODO
static const size_t RAM_SIZE = 5 * 1024; // RAM
static const size_t CPU_MEM = 64 * 1024; // Total address space
static const size_t VIDEO_MEM = 256 * 1024; // AED 512

static std::vector<std::string> roms = {
    "rom/aed_v86_c0.bin",
    "rom/aed_v86_c8.bin",
    "rom/aed_v86_d0.bin",
    "rom/aed_v86_d8.bin",
    "rom/aed_v86_e0.bin",
    "rom/aed_v86_e8.bin",
    "rom/aed_v86_f0.bin",
    "rom/aed_v86_f8.bin"
};

AedBus::AedBus() : _mapper(0, CPU_MEM), _videoMemory(VIDEO_MEM, 0xff) {
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
    _mapper.add(new M68B21(0x00, "PIA0"));
    _mapper.add(new M68B21(0x04, "PIA1"));
    _mapper.add(new M68B21(0x08, "PIA2"));
    _mapper.add(new Fuzz(0x2a, 1, "FUZZ"));
    _mapper.add(new Rom(0x10000 - romBuffer.size(), romBuffer));
    _mapper.add(new Ram(RAM_START, RAM_SIZE - RAM_START));
    _mapper.add(new Unknown(0, CPU_MEM));

    std::cerr << std::hex; // dump in hex
    std::cerr << _mapper;
}

AedBus::~AedBus() {
}

