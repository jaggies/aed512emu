/*
 * mandl.h
 *
 *  Created on: Sep 13, 2018
 *      Author: jmiller
 *
 *  Renders the Mandelbrot overview.
 */
#include <iostream>
#include "aedsequence.h"

const int xres = 512;
const int yres = 480;
const int maxCount = 256;

void mandel(double xmin, double xmax, double ymin, double ymax) {
    float cr_delta = (xmax - xmin)/(xres-1);
    float ci_delta = (ymax - ymin)/(yres-1);

    float ci = ymin;
    AedSequence seq;

    for (int j = 0; j < yres; j++, ci += ci_delta) {
        float cr = xmin;
        for (int i = 0; i < xres; i++, cr += cr_delta) {
            float zr = 0;
            float zi = 0;
            int count = 0;
            do {
                // z = z^2 + c
                // (zr + zi)*(zr + zi) = zr*zr + 2*zr*zi - zi*zi
                float tr = zr * zr - zi * zi;
                float ti = 2.0f * zr * zi;
                zr = tr + cr;
                zi = ti + ci;
            } while ((count++ < maxCount) && (zr*zr + zi*zi) < 4.0f);
            seq.mov(i,j).pixel(count).send([](int c) { fputc(c, stdout); });
        }
        fputc('\n', stdout);
    }
}

int main(int argc, char **argv)
{
    mandel(-2.0, 1.0, -1.25, 1.25);
}
