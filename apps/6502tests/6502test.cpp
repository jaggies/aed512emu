#include <iostream>
#include <fstream>
#include <tuple>
#include "dis6502.h"
#include "cpu6502.h"
#include "mapper.h"
#include "clk.h"
#include "bus.h"
#include "rom.h"
#include "ram.h"

#define CPU_MEM 65536

const std::vector<std::string> roms = { "rom/6502_functional_test.bin" };

class Clock: public CLK {
    public:
        Clock() : _count(0) { }
        ~Clock() = default;
        void add_cpu_cycles(size_t cycles) {
            _count += cycles;
        }
        size_t getCount() const {
            return _count;
        }
        void reset() {
            _count = 0;
        }
    private:
        size_t _count;
};

class System: public BUS {
    public:
        System() : _mapper(0, CPU_MEM) {
            // Open all ROM files and copy to ROM location in romBuffer
            std::vector<uint8_t> romBuffer;
            size_t offset = 0;
            for (const std::string& romFileName : roms) {
                std::ifstream file(romFileName, std::ios::binary | std::ios::ate);
                std::streamsize size = file.tellg();
                file.seekg(0, std::ios::beg);
                romBuffer.resize(romBuffer.size() + size);
                if (file.read((char*) &romBuffer[offset], size)) {
                    offset += size;
                }
            }

            // Add peripherals. Earlier peripherals are favored when addresses overlap.
            _mapper.add(new Rom(0x10000 - romBuffer.size(), romBuffer, true));
            std::cerr << std::hex; // dump in hex
            std::cerr << _mapper;
        }
        uint8_t read(int addr) override {
            return _mapper.read(addr);
        }
        void write(int addr, unsigned char value) override {
            _mapper.write(addr, value);
        }

    private:
        Mapper _mapper;
};

int main(int argc, char** argv) {
    Clock clock;
    System system;
    CPU6502<Clock, BUS> cpu(CPU6502<Clock, BUS>(clock, system));
    int lastpc = cpu.get_pc();
    cpu.set_pc(0x0400); // force start of program
    while (1) {
        std::string line;
        int count;
        int pc = cpu.get_pc();
        cpu.cycle();
        if (lastpc == pc) {
            std::cerr << "Failed at pc: " << pc << std::endl;
            cpu.dump(std::cerr);
            break;
        } else {
            std::tie(count, line) = disassemble_6502(pc,
                [&system](int offset) { return system.read(offset); }
            );
            std::cerr << line << "\n";
        }
        lastpc = pc;
    }
}
