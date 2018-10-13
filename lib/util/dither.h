/*
 * dither.h
 *
 *  Created on: Oct 13, 2018
 *      Author: jmiller
 */

#ifndef LIB_UTIL_DITHER_H_
#define LIB_UTIL_DITHER_H_

#include <cstdlib>

class Dither {
    public:
        Dither(size_t rbits, size_t gbits, size_t bbits) : _rbits(rbits), _gbits(gbits), _bbits(bbits) { }
        virtual ~Dither() = default;
        virtual int dither(int r, int g, int b, int x, int y) = 0;
    protected:
        size_t rbits() const { return _rbits; }
        size_t gbits() const { return _gbits; }
        size_t bbits() const { return _bbits; }
    private:
        size_t _rbits;
        size_t _gbits;
        size_t _bbits;
};

#endif /* LIB_UTIL_DITHER_H_ */
