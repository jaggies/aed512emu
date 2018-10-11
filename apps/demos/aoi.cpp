/*
 * aoi.cpp
 *
 *  Created on: Oct 10, 2018
 *      Author: jmiller
 */

#include "demo.h"

const int WIDTH = 50;
const int HEIGHT = 50;
const int WHITE = 7; // white pixel
const int BLACK = 0; // black pixel

void drawPixels(int w, int h, int zoom) {
    AedSequence seq;
    int xmin = (512 - w) / 2;
    int ymin = (512 - h) / 2;
    seq.mov(xmin, ymin).define_area_of_interest(xmin + w - 1, ymin + h - 1);
    uint8_t buffer[w*h];
    for (int j = 0; j < w; j++) {
        for (int i = 0; i < w; i++) {
            buffer[j*w + i] = ((i^j) & 1) ? WHITE : BLACK;
        }
    }
    seq.write_horizontal_scan_aoi(buffer, sizeof(buffer)).send();
}

int main(int argc, char **argv)
{
    drawPixels(WIDTH, HEIGHT, 1);
}
