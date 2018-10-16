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
inline void initLut(AedSequence& seq, int rbits, int gbits, int bbits) {
    assert(rbits + gbits + bbits <= 8);
    for (int b = 0; b < (1 << bbits); b++) {
        for (int g = 0; g < (1 << gbits); g++) {
            for (int r = 0; r < (1 << rbits); r++) {
                int idx = (b << (rbits + gbits)) | (g << rbits) | r;
                int red = r << (8-rbits);
                int grn = g << (8-gbits);
                int blu = b << (8-bbits);
                seq.set_color_table(idx, red, grn, blu).send();
            }
        }
    }
}

// Initializes color LUT with given allocation of bits.
inline void initGrayLut(AedSequence& seq) {
    for (int i = 0; i < 256; i++) {
        seq.set_color_table(i, i,i,i).send();
    }
}

#endif /* APPS_DEMOS_DEMO_H_ */
