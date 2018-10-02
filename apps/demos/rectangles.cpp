/*
 * rectangles.cpp
 *
 *  Created on: Sep 26, 2018
 *      Author: jmiller
 */

#include <cstdint>
#include "demo.h"

const int xres = 512;
const int yres = 480;

int main(int argc, char **argv) {
    AedSequence seq;
    initLut(3, 3, 2);
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


