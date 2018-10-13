/*
 * threshold.h
 *
 *  Created on: Oct 13, 2018
 *      Author: jmiller
 */

#ifndef LIB_UTIL_THRESHOLD_H_
#define LIB_UTIL_THRESHOLD_H_

#include "dither.h"

class Threshold: public Dither {
    public:
        Threshold(size_t rbits, size_t gbits, size_t bbits) : Dither(rbits, gbits, bbits) { }
        virtual ~Threshold() = default;
        int dither(int r, int g, int b, int x, int y) override;
};

#endif /* LIB_UTIL_THRESHOLD_H_ */
