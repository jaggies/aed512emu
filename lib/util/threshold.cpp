/*
 * threshold.cpp
 *
 *  Created on: Oct 13, 2018
 *      Author: jmiller
 */

#include "threshold.h"

int Threshold::dither(int red, int green, int blue, int x, int y) {
    const int rround = (1 << (rbits() - 1)) - 1;
    const int ground = (1 << (gbits() - 1)) - 1;
    const int bround = (1 << (bbits() - 1)) - 1;
    int rx = (red + rround) >> (8-rbits());
    int gx = (green + ground) >> (8-gbits());
    int bx = (blue + bround) >> (8-bbits());
    return (bx << (gbits() + rbits())) | (gx << rbits()) | rx;
}
