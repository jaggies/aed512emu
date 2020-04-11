/*
 * showpbm.cpp
 *
 *  Created on: Oct 13, 2018
 *      Author: jmiller
 */

#include <map>
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
static AedSequence seq;
typedef std::map<uint32_t, uint32_t> Map;
typedef std::multimap<uint32_t, uint32_t> MultiMap;

static inline uint32_t compose(uint8_t r, uint8_t g, uint8_t b) {
    return (r << 16) | (g << 8) | b;
}

static inline void decompose(uint32_t clr, uint8_t& r, uint8_t& g, uint8_t& b) {
    r = (clr >> 16) & 0xff;
    g = (clr >> 8) & 0xff;
    b = clr & 0xff;
}

static void analyzeCB(void* clientData, int x, int y, unsigned char pixel[3]) {
    Map& map = *(Map*) clientData;
    map[compose(pixel[0] & 0xf0, pixel[1] & 0xf0, pixel[2] & 0xf0)]++;
}

static void ditherCB(void* clientData, int x, int y, unsigned char pixel[3]) {
    static int ylast = -1;
    if (ylast != y) {
        ylast = y;
        if (idx > 0) {
            seq.mov(0,511-y).write_horizontal_runs(buffer, idx).send();
        }
        idx = 0;
    }
    buffer[idx++] = dither->dither(pixel[0], pixel[1], pixel[2], x, 511-y);
}

static int dist(uint32_t a, uint32_t b) {
    uint8_t r1, g1, b1;
    decompose(a, r1, g1, b1);
    uint8_t r2, g2, b2;
    decompose(b, r2, g2, b2);
    uint32_t sum = 0;
    sum += r1 > r2 ? (r1-r2)*(r1-r2) : (r2-r1)*(r2-r1);
    sum += g1 > g2 ? (g1-g2)*(g1-g2) : (g2-g1)*(g2-g1);
    sum += b1 > b2 ? (b1-b2)*(b1-b2) : (b2-b1)*(b2-b1);
    return sum;
}

static void optimalDisplayCB(void* clientData, int x, int y, unsigned char pixel[3]) {
    static int ylast = -1;
    static int rerr, gerr, berr;
    MultiMap& map = *(MultiMap*) clientData;
    if (ylast != y) {
        ylast = y;
        if (idx > 0) {
            seq.mov(0,511-y).write_horizontal_runs(buffer, idx).send();
        }
        idx = 0;
    }
    const uint32_t color = compose(
            std::max(0, std::min(255,int(pixel[0])+rerr)),
            std::max(0, std::min(255,int(pixel[1])+gerr)),
            std::max(0, std::min(255,int(pixel[2])+berr)));
    uint32_t bestIndex = 0;
    uint32_t bestColor = map.begin()->second;
    uint32_t bestDist = dist(color, bestColor); // pick the first color to start
    int index = 0;
    for (auto iter = map.begin(); iter != map.end(); iter++) {
        uint32_t d = dist(color, iter->second);
        if (d < bestDist) {
            bestIndex = index;
            bestColor = iter->second;
            bestDist = d;
        }
        index++;
    }
    buffer[idx++] = bestIndex;
    uint8_t r, g, b;
    decompose(bestColor, r, g, b);
    rerr += (int) pixel[0] - (int) r;
    gerr += (int) pixel[1] - (int) g;
    berr += (int) pixel[2] - (int) b;
}

void usage(const char* name) {
    std::cerr << "Usage: " << name
            << " -d(iffusion) | -t(hreshold) | -o(rdered) -O(ptimized)> <image.pbm>" << std::endl;
}

int main(int argc, char** argv) {
    NetPBM* pbm = createNetPBM();
    bool optimize = false;
    int opt;
    while ((opt = getopt(argc, argv, "Odot")) != -1) {
        switch (opt) {
            case 'd': // error diffusion
                dither = new DiffusionDither(RBITS, GBITS, BBITS);
            break;
            case 'o': // ordered dither
                dither = new OrderedDither(RBITS, GBITS, BBITS);
            break;
            case 't': // threshold dither (no dither)
                dither = new Threshold(RBITS, GBITS, BBITS);
            break;
            case 'O': // optimize image
                optimize = true;
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
    std::cerr << "Loading " << argv[optind] << " : " << pbm->width << "x" << pbm->height << std::endl;
    seq.set_zoom_register(1,1).send();

    buffer = new uint8_t[width];
    Map colorMap;

    if (!optimize) {
        initLut(seq, 3, 3, 2);
        pbm->read(pbm, ditherCB, nullptr);
    } else {
        // Find the 256 most common colors
        pbm->read(pbm, analyzeCB, &colorMap);
        MultiMap mm;
        for (auto iter = colorMap.begin(); iter != colorMap.end(); iter++) {
            mm.insert(mm.begin(), std::pair<uint32_t, uint32_t>(iter->second, iter->first));
        }
        if (mm.size() > 256) {
            auto start = mm.begin();
            std::advance(start, mm.size() - 256);
            mm.erase(mm.begin(), start);
        }

        uint8_t i = 0;
        for (const std::pair<uint32_t, uint32_t>& pr : mm) {
            uint8_t r, g, b;
            decompose(pr.second, r, g, b);
            seq.set_color_table(i++, r, g, b);
        }

        std::cerr << colorMap.size() << " unique colors" << std::endl;
        std::cerr << "After reduction: " << mm.size() << " colors" << std::endl;

        seq.mov(0,0).define_area_of_interest(width, height).send();

        pbm->reset(pbm);
        pbm->read(pbm, optimalDisplayCB, &mm);
    }
}


