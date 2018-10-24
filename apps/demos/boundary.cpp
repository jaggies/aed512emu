/*
 * boundary.cpp
 *
 *  Created on: Oct 22, 2018
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
    int n = 128;
    while(n--) {
		seq.mov(random() % xres, random() % yres);
		seq.color(random() % 256);
		seq.circle(1 + (random() & 0x7e), true);
		seq.boundary_fill(random() & 0xff);
		seq.send();
    }
}

