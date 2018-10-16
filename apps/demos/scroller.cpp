/*
 * scroller.cpp
 *
 *  Created on: Oct 16, 2018
 *      Author: jmiller
 */

#include "demo.h"

const int xres = 512;
const int yres = 480;

int main(int argc, char **argv) {
    AedSequence seq;
    for (int x = 0; x < 512; x++) {
        seq.scroll_relative(1, 0).send();
    }

    for (int y = 0; y < 512; y++) {
        seq.scroll_relative(0, 1).send();
    }
}


