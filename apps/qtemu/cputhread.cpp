/*
 * CpuThread.cpp
 *
 *  Created on: Oct 14, 2018
 *      Author: jmiller
 */

#include "cputhread.h"

#include "cpu6502.h"
#include "mos6502.h"
#include "dis6502.h"
#include "config.h"
#include "aedsequence.h"

const int CPU_MHZ = 2000000;

// These need to be static because they're passed asynchronously to the UI thread.
// TODO: eliminate this dependency.
 static int zoom[2];
 static int scroll[2];

CpuThread::CpuThread(QObject *parent): QThread(parent) {

    _clk = new Clock(CPU_MHZ);

    _bus = new AedBus(
           [this]() { _cpu->irq(); }, [this]() { _cpu->nmi(); },
           [this]() {
               int width; int height;
               const std::vector<uint8_t>& mem = _bus->getRawVideo(&width, &height);
               const uint8_t* red;
               const uint8_t* green;
               const uint8_t* blue;
               _bus->getLut(&red, &green, &blue);
               _bus->getZoom(&zoom[0], &zoom[1]);
               _bus->getScroll(&scroll[0], &scroll[1]);
               emit signal_redraw(&mem[0], red, green, blue, zoom, scroll, width, height);
           });

    _cpu = new USE_CPU(
           [this](int addr) { return _bus->read(addr); },
           [this](int addr, uint8_t value) { _bus->write(addr, value); },
           [this](int cycles) { _clk->add_cpu_cycles(cycles); _bus->setCpuTime(_clk->getCpuTime()); },
           [this](CPU::ExceptionType ex, int pc) { emit signal_exception(ex, pc); });
}

void
CpuThread::run() {
    while (!_flag_stop) {
        _cpu->cycle(2);
        _bus->handleEvents(_clk->getCpuTime());
        usleep(1);
    }
}

