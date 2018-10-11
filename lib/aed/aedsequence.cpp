/*
 * aedsequence.cpp
 *
 *  Created on: Sep 24, 2018
 *      Author: jmiller
 */

#include <cassert>
#include "aedsequence.h"

void
AedSequence::write_horizontal_run(const uint8_t* begin, const uint8_t* end, State state) {
    assert(end > begin);
    int count = end - begin;
    if (state == RLE) {
        while (count > 0) {
            int n = std::min(254, count);
            assert(n > 0);
            _sequence.push_back(n); // # of same color
            _sequence.push_back(*begin); // the color
            count -= n;
        }
    } else if (state == UNIQ) {
        while (count > 0) {
            int n = std::min(254, count);
            assert(n > 0);
            _sequence.push_back(255); // buffer of unique pixels
            _sequence.push_back(n); // number of colors
            _sequence.insert(_sequence.end(), begin, begin + n);
            begin += n;
            count -= n;
        }
    }
}

AedSequence& AedSequence::write_horizontal_scan_aoi(const uint8_t* buffer, int n) {
    _sequence.push_back(WHS);
    _sequence.insert(_sequence.end(), buffer, buffer + n);
    return *this;
}

AedSequence& AedSequence::write_horizontal_runs(const uint8_t* buffer, int n) {
    const uint8_t* begin = buffer;
    const uint8_t* current = begin + 1;
    const uint8_t* end = &buffer[n];

    State state = SEARCH;
    _sequence.push_back(WHU);
    while (current < end) {
        //std::cout << state << " " << *begin << std::endl;
        if (state == SEARCH) {
            if (*current != *begin) {
                state = UNIQ;
            } else {
                state = RLE;
            }
            continue;
        } else if (state == RLE) {
            if (*begin != *current) {
                write_horizontal_run(begin, current, RLE);
                begin = current;
                state = SEARCH;
            }
        } else if (state == UNIQ) {
            if (*current == *(current+1)) {
                write_horizontal_run(begin, current, UNIQ);
                begin = current;
                state = SEARCH;
            }
        }
        current++;
    }
    write_horizontal_run(begin, current, state);
    _sequence.push_back(0); // termination
    return *this;
}
