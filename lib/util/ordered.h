/*
 * ordered.h
 *
 *  Created on: Oct 13, 2018
 *      Author: jmiller
 */

#ifndef LIB_UTIL_ORDERED_H_
#define LIB_UTIL_ORDERED_H_

#include "dither.h"

class OrderedDither : public Dither {
    public:
        OrderedDither(size_t rbits, size_t gbits, size_t bbits) : Dither(rbits, gbits, bbits) { }
        virtual ~OrderedDither() = default;
        int dither(int r, int g, int b, int x, int y) override;
    private:
        int single(int inLevels, int outLevels, int x, int y, int grey);
};

#endif /* LIB_UTIL_ORDERED_H_ */
