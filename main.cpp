#include <iostream>
#include "cpu6502.h"
#include "clk.h"
#include "bus.h"
#include "aedbus.h"

class Clock : public CLK {
    public:
        Clock() : _count(0) { }
        ~Clock() = default;
        void add_cpu_cycles(size_t cycles) { }
        size_t getCount() const { return _count; }
        void reset() { _count = 0; }
    private:
        size_t _count;
};

int main(int argc, char** argv)
{
    Clock clock;
    AedBus bus;
    CPU6502<Clock, AedBus> cpu(CPU6502<Clock, AedBus>(clock, bus));
    cpu.reset();
    while (1) {
        cpu.cycle();
        if (!(clock.getCount() % 20000)) {
            // TODO: automate this with a signal handler. Should operate at 60Hz.
            bus.doVideoIrq();
        }
    }
}
