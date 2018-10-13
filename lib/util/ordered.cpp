/*
 * ordered.cpp
 *
 *  Created on: Oct 13, 2018
 *      Author: jmiller
 */

#include "ordered.h"

/*
 * dither.cpp
 *
 *  Created on: Oct 13, 2018
 *      Author: jmiller
 */

#include "dither.h"

static const int matrix[4][4] = {
    { 0, 12, 3, 15 },
    { 8, 4, 11, 7 },
    { 2, 14, 1, 13 },
    { 10, 6, 9, 5 }};

int OrderedDither::single(int inLevels, int outLevels, int x, int y, int grey) {
    int thresh, val, err, n;
    // the threshold for the decision
    thresh = matrix[x & 3][y & 3];

    // Lower of the two possible values, due to integer division
    val = grey * (outLevels - 1) / inLevels;

    // Error for choosing this value
    err = grey - val * inLevels / (outLevels - 1);

    // Calculate normalized value between 0 and 15 for given error
    n = 16 * err * outLevels / inLevels;

    return (n > thresh) ? (val + 1) : val;
}

int OrderedDither::dither(int red, int green, int blue, int x, int y) {
    int rx = single(1 << 8, 1 << rbits(), x, y, red);
    int gx = single(1 << 8, 1 << gbits(), x, y, green);
    int bx = single(1 << 8, 1 << bbits(), x, y, blue);
    return (bx << (gbits() + rbits())) | (gx << rbits()) | rx;
}



