#include <iostream>
#include "cpu6502.h"
#include "mos6502.h"
#include "clk.h"
#include "bus.h"
#include "aedbus.h"
#include "netpbm.h"
#include "config.h"

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

void writeFrame(const std::string& path, AedBus& bus) {
    NetPBM* pbm = createNetPBM();
    assert(path.size() > 0);
    int depth = 255;
    int width = bus.getDisplayWidth();
    int height = bus.getDisplayHeight();
    if (path.size() == 0 || !pbm->open(pbm, path.c_str(), &width, &height, &depth, NETPBM_WRITE)) {
       std::cerr << "Can't write image " <<  path << std::endl;
       return;
    }
    for (int h = height - 1; h >= 0; h--) {
       for (size_t w = 0; w < width; w++) {
           uint32_t pixel = bus.getPixel(w, h);
           uint8_t rgb[] = { uint8_t(pixel & 0xff), uint8_t((pixel >> 8) & 0xff), uint8_t((pixel >> 16) & 0xff) };
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
    USE_CPU cpu([&bus](int addr) { return bus.read(addr); },
                [&bus](int addr, uint8_t value) { bus.write(addr, value); },
                [&clock](int cycles) { clock.add_cpu_cycles(cycles); });

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
                writeFrame(path, bus);
            }
        }
        if (bus.doSerial()) {
            cpu.irq();
        }
    }
}
