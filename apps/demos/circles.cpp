/*
 * circles.cpp
 *
 *  Created on: Sep 26, 2018
 *      Author: jmiller
 */

#include <iostream>
#include <unistd.h>
#include "aedsequence.h"

const int xres = 512;
const int yres = 480;
const int RBITS = 3;
const int GBITS = 3;
const int BBITS = 2;

void drawCircle(uint16_t x, uint16_t y, uint8_t color) {
    AedSequence seq;
    seq.mov(x, y).circle(color).send();
}

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
    seq.setlut(0, 0, 0, 255);
    seq.setlut(1, 255, 255, 255);
    seq.mov(256,256);
    seq.circle(100);
    seq.send();
    int n = 20;
    while(n--) {
        seq.mov(random() % xres, random() % yres);
        seq.circle(random(), true);
        seq.send();
    }
}

