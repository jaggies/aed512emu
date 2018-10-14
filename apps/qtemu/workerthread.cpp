/*
 * WorkerThread.cpp
 *
 *  Created on: Oct 14, 2018
 *      Author: jmiller
 */

#include "cpu6502.h"
#include "mos6502.h"
#include "dis6502.h"
#include "config.h"
#include "aedsequence.h"
#include "workerthread.h"

const int CPU_MHZ = 2000000;

WorkerThread::WorkerThread(QObject *parent): QThread(parent) {
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
               emit handleRedraw(&mem[0], red, green, blue, width, height);
           });


    _cpu = new USE_CPU(
           [this](int addr) { return _bus->read(addr); },
           [this](int addr, uint8_t value) { _bus->write(addr, value); },
           [this](int cycles) { _clk->add_cpu_cycles(cycles); _bus->setCpuTime(_clk->getCpuTime()); },
           [this](CPU::ExceptionType ex, int pc) { emit handleException(ex, pc); });
}

void
WorkerThread::run() {
    while (!_flag_stop) {
        _cpu->cycle(1);
        _bus->handleEvents(_clk->getCpuTime());
        usleep(2);
    }
}

