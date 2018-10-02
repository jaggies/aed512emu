/*
 * demo.h
 *
 *  Created on: Oct 2, 2018
 *      Author: jmiller
 */

#ifndef APPS_DEMOS_DEMO_H_
#define APPS_DEMOS_DEMO_H_

#include <cassert>
#include "aedsequence.h"

// Initializes color LUT with given allocation of bits.
inline void initLut(int rbits, int gbits, int bbits) {
    AedSequence seq;
    assert(rbits + gbits + bbits <= 8);
    for (int b = 0; b < (1 << bbits); b++) {
        for (int g = 0; g < (1 << gbits); g++) {
            for (int r = 0; r < (1 << rbits); r++) {
                int idx = (b << (rbits + gbits)) | (g << rbits) | r;
                int red = r << (8-rbits);
                int grn = g << (8-gbits);
                int blu = b << (8-bbits);
                seq.setlut(idx, red, grn, blu).send();
            }
        }
    }
}

inline void drawCircle(uint16_t x, uint16_t y, uint8_t color) {
    AedSequence seq;
    seq.mov(x, y).circle(color).send();
}

#endif /* APPS_DEMOS_DEMO_H_ */
