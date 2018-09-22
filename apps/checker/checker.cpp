#include <iostream>
#include <fstream>
#include <tuple>
#include "dis6502.h"
#include "cpu6502.h"
#include "mos6502.h"
#include "mapper.h"
#include "clk.h"
#include "bus.h"
#include "rom.h"
#include "ram.h"
#include "config.h"

#define CPU_MEM 65536 // This test uses entire address space

const std::vector<std::string> roms = { "rom/6502_functional_test.bin" };

typedef CLK Clock;

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
    Clock clock(1000000); // 1MHz system clock
    System system;
    USE_CPU cpu([&system](int addr) { return system.read(addr); },
            [&system](int addr, uint8_t value) { system.write(addr, value); },
            [&clock](int cycles) { clock.add_cpu_cycles(cycles); });

    int lastpc = cpu.get_pc();
    cpu.set_pc(0x0400); // force start of program
    while (1) {
        std::string line;
        int pc = cpu.get_pc();
        cpu.cycle();
        if (lastpc == pc) {
            std::cerr << "Failed at pc: " << pc << std::endl;
            int count;
            std::tie(count, line) = disassemble_6502(&pc,
                [&system](int offset) { return system.read(offset); }
            );
            std::cerr << line << "\n";
            cpu.dump(std::cerr);
            break;
        }
        lastpc = pc;
    }
}
