/*
 * bitplanes.cpp
 *
 *  Created on: Oct 16, 2018
 *      Author: jmiller
 */

#include "demo.h"
#include "aedsequence.h"

const int xres = 512;
const int yres = 480;

int main(int argc, char **argv) {
    AedSequence seq;
    initGrayLut(seq);
    seq.clear().send();
    const int height = yres / 8;
    for (int i = 0; i < 8; i++) {
        int dacValue = 255*i / 7;
        seq.set_color_table(1 << i, dacValue, dacValue, dacValue).send();
        seq.color(1 << i).mov(0, i*height).rectangle(xres, (i+1)*height).send();
    }
}
