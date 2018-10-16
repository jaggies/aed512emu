/*
 * mandl.h
 *
 *  Created on: Sep 13, 2018
 *      Author: jmiller
 *
 *  Renders the Mandelbrot overview.
 */
#include <iostream>
#include <cmath>
#include "demo.h"

const int xres = 512;
const int yres = 512;
const int maxCount = 1024;
static AedSequence seq;
const float centerX = -0.722;
const float centerY = 0.244095;
const float width = 0.00019;

void mandel(double xmin, double xmax, double ymin, double ymax) {
    float cr_delta = (xmax - xmin)/(xres-1);
    float ci_delta = (ymax - ymin)/(yres-1);
    float ci = ymin;

    uint8_t buffer[xres];
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
            buffer[i] = count % 256;
        }
        seq.mov(0, j).write_horizontal_runs(buffer, xres).send();
    }
}

float f(float x) {
    return 127 + 127*cos(x * M_PI);
}

int main(int argc, char **argv)
{
    for (int i = 0; i < 256; i++) {
        seq.setlut(i, f(i / 256.0), f((i+128)/256.0), f((i+1192)/256.0));
    }
    mandel(centerX - width/2, centerX + width/2, centerY - width/2, centerY + width/2);
}
