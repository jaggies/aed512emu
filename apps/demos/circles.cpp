/*
 * circles.cpp
 *
 *  Created on: Sep 26, 2018
 *      Author: jmiller
 */

#include <iostream>
#include <unistd.h>
#include "demo.h"

const int xres = 512;
const int yres = 480;

int main(int argc, char **argv) {
    AedSequence seq;
    initLut(seq, 3, 3, 2);
    int n = 100;
    while(n--) {
        seq.mov(random() % xres, random() % yres);
        seq.color(random() % 256);
        seq.circle(random() & 0x7f, true);
        seq.send();
    }
}

