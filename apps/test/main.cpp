#include <iostream>
#include "cpu6502.h"
#include "mos6502.h"
#include "clk.h"
#include "bus.h"
#include "aedbus.h"
#include "netpbm.h"

class Clock : public CLK {
    public:
        Clock() : _count(0) { }
        ~Clock() = default;
        void add_cpu_cycles(size_t cycles) { _count += cycles; }
        size_t getCount() const { return _count; }
        void reset() { _count = 0; }
    private:
        size_t _count;
};

void writeFrame(const std::string& path, int width, int height, const std::vector<uint8_t>& mem) {
    NetPBM* pbm = createNetPBM();
    assert(path.size() > 0);
    int depth = 255;
    if (path.size() == 0 || !pbm->open(pbm, path.c_str(), &width, &height, &depth, NETPBM_WRITE)) {
       std::cerr << "Can't write image " <<  path << std::endl;
       return;
    }
    for (int h = height - 1; h >= 0; h--) {
       for (size_t w = 0; w < width; w++) {
           unsigned char rgb[3];
           // TODO: use CLUT
           rgb[0] = rgb[1] = rgb[2] = mem[h*width + w] ? 0xff : 0;
           pbm->write(pbm, rgb);
       }
    }
    pbm->close(pbm);
    free(pbm); // TODO: cleanup
}

int main(int argc, char** argv)
{
    Clock clock;
    AedBus bus;
//    CPU6502 cpu([&bus](int addr) { return bus.read(addr); },
//            [&bus](int addr, uint8_t value) { bus.write(addr, value); },
//            [&clock](int cycles) { clock.add_cpu_cycles(cycles); });
    mos6502 cpu([&bus](int addr) { return bus.read(addr); },
                [&bus](int addr, uint8_t value) { bus.write(addr, value); },
                [&clock](int cycles) { clock.add_cpu_cycles(cycles); });
    cpu.reset();
    bus.send("Hello world!!!\n");
    int frameCount = 0;
    while (1) {
        for (int i = 0; i < 1000000; i++) {
            cpu.cycle();
        }
        // TODO: automate this with a signal handler. Should operate at 60Hz.
        if (bus.doVideo()) {
            cpu.nmi();
            if (true) {
                std::string path = "frame" + std::to_string(frameCount++);
                writeFrame(path, bus.getDisplayWidth(), bus.getDisplayHeight(), bus.getVideoMemory());
            }
        }
        if (bus.doSerial()) {
            cpu.irq();
        }
    }
}
