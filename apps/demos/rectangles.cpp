/*
 * rectangles.cpp
 *
 *  Created on: Sep 26, 2018
 *      Author: jmiller
 */

#include <cstdint>
#include "aedsequence.h"

const int xres = 512;
const int yres = 480;
const int RBITS = 3;
const int GBITS = 3;
const int BBITS = 2;

void initLut() {
    AedSequence seq;
    for (int b = 0; b < (1 << BBITS); b++) {
        for (int g = 0; g < (1 << GBITS); g++) {
            for (int r = 0; r < (1 << RBITS); r++) {
                int idx = (b << (RBITS + GBITS)) | (g << RBITS) | r;
                int red = r << (8-RBITS);
                int grn = g << (8-GBITS);
                int blu = b << (8-BBITS);
                seq.setlut(idx, red, grn, blu).send();
            }
        }
    }
}

int main(int argc, char **argv) {
    AedSequence seq;
    initLut();
    int n = 10;
    while(n--) {
        uint16_t x1 = random() % xres;
        uint16_t y1 = random() % yres;
        uint16_t x2 = random() % xres;
        uint16_t y2 = random() % yres;
        if (x1 > x2) std::swap(x1, x2);
        if (y1 > y2) std::swap(y1, y2);
        seq.color(random() % 256);
        seq.mov(x1, y1).rectangle(x2, y2).send();
    }
}


