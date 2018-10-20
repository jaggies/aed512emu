/*
 * showpbm.cpp
 *
 *  Created on: Oct 13, 2018
 *      Author: jmiller
 */

#include <getopt.h>
#include "demo.h"
#include "netpbm.h"
#include "ordered.h"
#include "diffusion.h"
#include "threshold.h"

const int RBITS = 3;
const int GBITS = 3;
const int BBITS = 2;

static Dither* dither;

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
    buffer[idx++] = dither->dither(pixel[0], pixel[1], pixel[2], x, 511-y);
}

void usage(const char* name) {
    std::cerr << "Usage: " << name << " -d <ordered|diffusion|threshold> <image.pbm>" << std::endl;
}

int main(int argc, char** argv) {
    NetPBM* pbm = createNetPBM();
    int opt;
    while ((opt = getopt(argc, argv, "d:")) != -1) {
       switch(opt) {
           case 'd':
               switch(*optarg) {
                   case 'd': // error diffusion
                       dither = new DiffusionDither(RBITS, GBITS, BBITS);
                   break;
                   case 'o': // ordered dither
                       dither = new OrderedDither(RBITS, GBITS, BBITS);
                   break;
                   case 't': // threshold dither (no dither)
                       dither = new Threshold(RBITS, GBITS, BBITS);
                   break;
               }
           break;
       }
    }

    if (!argv[optind]) {
        usage(argv[0]);
        exit(0);
    }

    if (!dither) {
        dither = new OrderedDither(RBITS, GBITS, BBITS);
    }

    width = height = depth = 0;
    if (!pbm->open(pbm, argv[optind], &width, &height, &depth, NETPBM_READ)) {
        std::cerr << "Can't open %s\n" << argv[1] << std::endl;
        return 0;
    }
    std::cerr << "Loading " << argv[1] << " : " << pbm->width << "x" << pbm->height << std::endl;
    AedSequence seq;
    seq.set_zoom_register(1,1).send();
    initLut(seq, 3, 3, 2);

    buffer = new uint8_t[width];
    seq.mov(0,0).define_area_of_interest(width, height).send();
    pbm->read(pbm, callback, &seq);
}


