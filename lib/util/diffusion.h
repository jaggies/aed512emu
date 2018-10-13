/*
 * diffusion.h
 *
 *  Created on: Oct 13, 2018
 *      Author: jmiller
 */

#ifndef LIB_UTIL_DIFFUSION_H_
#define LIB_UTIL_DIFFUSION_H_

#include "dither.h"

class DiffusionDither: public Dither {
    public:
        DiffusionDither(size_t rbits, size_t gbits, size_t bbits) : Dither(rbits, gbits, bbits) { }
        virtual ~DiffusionDither() = default;
        int dither(int red, int green, int blue, int x, int y) override;
    private:
        int single(int inLevels, int outLevels, int grey, int* err);
        int _rerr = 0;
        int _gerr = 0;
        int _berr = 0;
};

#endif /* LIB_UTIL_DIFFUSION_H_ */
