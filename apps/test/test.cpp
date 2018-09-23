#include <iostream>
#include "cpu6502.h"
#include "mos6502.h"
#include "clk.h"
#include "bus.h"
#include "aedbus.h"
#include "config.h"

typedef CLK Clock;

int main(int argc, char** argv)
{
    Clock clock(1000000); // 1MHz system clock
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
        if (bus.doVideo(clock.getCpuTime())) {
            cpu.nmi();
            if (true) {
                std::string path = "frame" + std::to_string(frameCount++);
                bus.saveFrame(path);
            }
        }
        if (bus.doSerial()) {
            cpu.irq();
        }
    }
}
