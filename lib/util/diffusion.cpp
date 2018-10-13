/*
 * diffusion.cpp
 *
 *  Created on: Oct 13, 2018
 *      Author: jmiller
 */

#include "diffusion.h"

int DiffusionDither::single(int inLevels, int outLevels, int grey, int* err) {
    int result = (grey + *err)*(outLevels-1) / inLevels; // Lower bound due to integer division
    *err += grey - result*inLevels/(outLevels-1); // Error for choosing this value
    return result;
}

int DiffusionDither::dither(int red, int green, int blue, int x, int y) {
    int rx = single(1 << 8, 1 << rbits(), red, &_rerr);
    int gx = single(1 << 8, 1 << gbits(), green, &_gerr);
    int bx = single(1 << 8, 1 << bbits(), blue, &_berr);
    return (bx << (gbits() + rbits())) | (gx << rbits()) | rx;
}

