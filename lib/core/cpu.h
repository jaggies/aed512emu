/*
 * cpu.h
 *
 *  Created on: Sep 18, 2018
 *      Author: jmiller
 */

#ifndef LIB_CORE_CPU_H_
#define LIB_CORE_CPU_H_

#include <ostream>
#include <vector>

//
// Abstract base class for a CPU
//
class CPU {
    public:
        enum ExceptionType { NONE = 0, ILLEGAL_INSTRUCTION, WATCH_POINT, BREAK_POINT };
        typedef std::function<uint8_t(int)> Reader;
        typedef std::function<void(int offset, uint8_t value)> Writer;
        typedef std::function<void(int cycles)> Counter;
        typedef std::function<void(ExceptionType ex, int pc)> Exception;

        CPU(Reader reader, Writer writer, Counter counter, Exception exception)
                : _reader(reader), _writer(writer), _counter(counter), _exception(exception),
                  _prev_pc(0) { }
        virtual ~CPU() = default;

        // Issues n cycles.
        void cycle(size_t cycles = 1) {
            while (cycles--) {
                _prev_pc = get_pc();
                if (std::find(_break.begin(), _break.end(), get_pc()) != _break.end()) {
                    exception(BREAK_POINT);
                    break;
                } else {
                    do_cycle();
                }
            }
        }

        virtual void irq() = 0;
        virtual void nmi() = 0;
        virtual void reset() = 0;

        virtual int get_pc() const = 0;

        virtual void dump(std::ostream& os) = 0;

        // Adds a breakpoint at the given PC
        virtual void addBreak(int pc) { _break.push_back(pc); }

        // Adds a watchpoint for the given memory location. Calls Exception
        virtual void addWatch(int addr) { _watch.push_back(addr); }

        // Gets list of breakpoints
        virtual const std::vector<uint32_t>& getBreakpoints() const { return _break; }

        // Gets list of watch points
        virtual const std::vector<uint32_t>& getWatchpoints() const { return _watch; }

        // For debug/testing only
        virtual void set_pc(int pc_) = 0;
        virtual uint8_t read_mem(int addr) {
            return read(addr);
        }
        virtual void write_mem(int addr, uint8_t data) {
            write(addr, data);
        }

    protected:
        virtual void do_cycle() = 0;

        uint8_t read(int address) {
            if (std::find(_watch.begin(), _watch.end(), address) != _watch.end()) {
                exception(WATCH_POINT);
            }
            return _reader(address);
        }
        void write(int address, uint8_t value) {
            if (std::find(_watch.begin(), _watch.end(), address) != _watch.end()) {
                exception(WATCH_POINT);
            }
            _writer(address, value);
        }
        void count(int cycles) { _counter(cycles); }

        // Tell the host there was an exception
        virtual void exception(ExceptionType ex) { _exception(ex, _prev_pc); }

    private:
        std::vector<uint32_t> _watch;
        std::vector<uint32_t> _break;
        Reader _reader;
        Writer _writer;
        Counter _counter;
        Exception _exception;
        int _prev_pc; // for exception handling
};

#endif /* LIB_CORE_CPU_H_ */
