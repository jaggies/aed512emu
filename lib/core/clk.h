/*
 * CLK.h
 *
 *  Created on: Sep 8, 2018
 *      Author: jmiller
 */

#ifndef CLK_H_
#define CLK_H_

#include <cstdint>
#include <cstdlib>
#include "coreutil.h"

class CLK {
    public:
        CLK(uint32_t hz) : _hz(hz), _cycleCount(0) { }
        virtual ~CLK() = default;
        virtual void add_cpu_cycles(size_t cycles) { _cycleCount += cycles; }
        virtual size_t getCycleCount() const { return _cycleCount; }
        virtual void reset() { _cycleCount = 0; }
        virtual uint32_t getFrequency() const { return _hz; }
        // Returns CPU time in microseconds (us)
        virtual uint64_t getCpuTime() const {
            return SECS2USECS(1) * _cycleCount / _hz;
        }
    private:
        const uint32_t _hz;
        size_t _cycleCount;
};

#endif /* CLK_H_ */
