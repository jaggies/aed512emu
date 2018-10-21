/*
 * zoom.cpp
 *
 *  Created on: Oct 16, 2018
 *      Author: jmiller
 */

#include <unistd.h>
#include "demo.h"

int main(int argc, char **argv) {
    AedSequence seq;
    for (int i = 0; i < 256; i++) {
        int zx = (i & 0xf) + 1;
        int zy = (i >> 4) + 1;
        seq.set_zoom_register(zx, zy).send();
		usleep(100000);
    }
    seq.set_zoom_register(1, 1).send();
}



