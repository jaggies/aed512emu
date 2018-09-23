/*
 * cpu.h
 *
 *  Created on: Sep 18, 2018
 *      Author: jmiller
 */

#ifndef LIB_CORE_CPU_H_
#define LIB_CORE_CPU_H_

#include <ostream>

//
// Abstract base class for a CPU
//
class CPU {
    public:
        enum ExceptionType { NONE = 0, ILLEGAL_INSTRUCTION };
        typedef std::function<uint8_t(int)> Reader;
        typedef std::function<void(int offset, uint8_t value)> Writer;
        typedef std::function<void(int cycles)> Counter;
        typedef std::function<void(ExceptionType ex)> Exception;

        CPU(Reader reader, Writer writer, Counter counter, Exception exception)
                : _reader(reader), _writer(writer), _counter(counter), _exception(exception) { }
        virtual ~CPU() = default;

        virtual void irq() = 0;
        virtual void nmi() = 0;
        virtual void reset() = 0;
        virtual void cycle() = 0;
        virtual int get_pc() const = 0;

        virtual void dump(std::ostream& os) = 0;

        // For debug/testing only
        virtual void set_pc(int pc_) = 0;

    protected:
        uint8_t read(int address) { return _reader(address); }
        void write(int address, uint8_t value) { _writer(address, value); }
        void count(int cycles) { _counter(cycles); }

        // Tell the host there was an exception
        virtual void exception(ExceptionType ex) { _exception(ex); }

    private:
        Reader _reader;
        Writer _writer;
        Counter _counter;
        Exception _exception;
};

#endif /* LIB_CORE_CPU_H_ */
