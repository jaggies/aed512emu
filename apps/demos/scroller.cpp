/*
 * scroller.cpp
 *
 *  Created on: Oct 16, 2018
 *      Author: jmiller
 */

#include <unistd.h>
#include "demo.h"

const int STEPS = 512;

int main(int argc, char **argv) {
    AedSequence seq;
    for (int x = 0; x < STEPS; x++) {
        seq.scroll_relative(1, 0).send();
		usleep(5000);
    }

    for (int y = 0; y < STEPS; y++) {
        seq.scroll_relative(0, 1).send();
		usleep(5000);
    }
}


