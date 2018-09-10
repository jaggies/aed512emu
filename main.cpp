#include <iostream>
#include "cpu6502.h"
#include "clk.h"
#include "bus.h"
#include "aedbus.h"

class Clock : public CLK {
    public:
        void add_cpu_cycles(size_t cycles) { }
};

int main(int argc, char** argv)
{
    Clock clock;
    AedBus bus;
    CPU6502<Clock, AedBus> cpu(CPU6502<Clock, AedBus>(clock, bus));
    cpu.reset();
    while (1) {
        cpu.cycle();
    }
}
