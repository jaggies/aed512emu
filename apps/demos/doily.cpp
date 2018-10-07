/*
 * doily.cpp
 *
 *  Created on: Oct 7, 2018
 *      Author: jmiller
 */

#include <iostream>
#include <cmath>
#include "demo.h"

const int xres = 483;
const int yres = xres;
const int points = 20;

int main(int argc, char **argv) {
    AedSequence seq;
    float x[points], y[points];

    for (int i = 0; i < points; i++) {
        x[i] = xres * 0.5f * (1.0f + cos(2.0f * M_PI * i / (points - 1)));
        y[i] = 42 + yres * 0.5f * (1.0f + sin(2.0f * M_PI * i / (points - 1)));
    }

    seq.color(7).send(); // white

    for (int n = 0; n < points; n++) {
        for (int m = n+1; m < points; m++) {
            seq.mov(x[n], y[n]).line(x[m], y[m]).send();
        }
    }
}



