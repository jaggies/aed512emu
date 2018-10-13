/*
 * showpbm.cpp
 *
 *  Created on: Oct 13, 2018
 *      Author: jmiller
 */

#include "demo.h"
#include "netpbm.h"
#include "ordered.h"
#include "diffusion.h"

const int RBITS = 3;
const int GBITS = 3;
const int BBITS = 2;

static DiffusionDither dither(RBITS, GBITS, BBITS);
static int width, height, depth;
static uint8_t* buffer;
static int idx;

static void callback(void* clientData, int x, int y, unsigned char pixel[3]) {
    static int ylast = -1;
    AedSequence& seq = *(AedSequence*) clientData;
    if (ylast != y) {
        ylast = y;
        if (idx > 0) {
            seq.mov(0,511-y).write_horizontal_runs(buffer, idx).send();
        }
        idx = 0;
    }
    buffer[idx++] = dither.dither(pixel[0], pixel[1], pixel[2], x, 511-y);
}

int main(int argc, char** argv) {
    NetPBM* pbm = createNetPBM();
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << "<image.pbm>" << std::endl;
        return 0;
    }
    width = height = depth = 0;
    if (!pbm->open(pbm, argv[1], &width, &height, &depth, NETPBM_READ)) {
        std::cerr << "Can't open %s\n" << argv[1] << std::endl;
        return 0;
    }
    std::cerr << "Loading " << argv[1] << " : " << pbm->width << "x" << pbm->height << std::endl;
    AedSequence seq;
    initLut(seq, 3, 3, 2);

    buffer = new uint8_t[width];
    seq.mov(0,0).define_area_of_interest(width, height).send();
    pbm->read(pbm, callback, &seq);
}


