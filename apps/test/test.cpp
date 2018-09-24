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
    USE_CPU * cpu = nullptr;
    AedBus bus([&cpu]() { cpu->irq(); }, [&cpu]() { cpu->nmi(); });
    cpu = new USE_CPU([&bus](int addr) { return bus.read(addr); },
                [&bus](int addr, uint8_t value) { bus.write(addr, value); },
                [&clock](int cycles) { clock.add_cpu_cycles(cycles); });

    bus.send("Hello world!!!\n");

    while (1) {
        cpu->cycle(1000);
        bus.handleEvents(clock.getCpuTime());
    }
}
