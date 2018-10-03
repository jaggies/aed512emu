/*
 * aedsequence.h
 *
 *  Created on: Sep 24, 2018
 *      Author: jmiller
 */

#ifndef LIB_AED_AEDSEQUENCE_H_
#define LIB_AED_AEDSEQUENCE_H_
#include <vector>
#include <functional>
#include <iostream>
#include "aedcmds.h"

class AedSequence {
    enum State { SEARCH, RLE, UNIQ }; // For run-length encoding
    public:
        AedSequence() = default;
        virtual ~AedSequence() = default;

        AedSequence& circle(int radius, bool fat = false) {
            _sequence.push_back(fat ? DFC : DCL);
            _sequence.push_back(radius & 0xff);
            return *this;
        }

        AedSequence& clear() {
            _sequence.push_back(ERS);
            return *this;
        }

        AedSequence& color(uint8_t clr) {
            _sequence.push_back(SEC);
            _sequence.push_back(clr);
            return *this;
        }

        AedSequence& fill(uint8_t clr) {
            _sequence.push_back(IFL);
            _sequence.push_back(clr);
            return *this;
        }

        AedSequence& mov(uint16_t x, uint16_t y) {
            _sequence.push_back(MOV);
            coordinate(x, y);
            return *this;
        }

        AedSequence& pixel(uint8_t color) {
            _sequence.push_back(WPX);
            _sequence.push_back(color);
            return *this;
        }

        AedSequence& rectangle(uint16_t x, uint16_t y) {
            _sequence.push_back(DFR);
            coordinate(x,y);
            return *this;
        }

        AedSequence& setlut(uint8_t index, uint8_t r, uint8_t g, uint8_t b) {
            _sequence.push_back(SCT);
            _sequence.push_back(index);
            _sequence.push_back(1); // just one entry
            _sequence.push_back(r);
            _sequence.push_back(g);
            _sequence.push_back(b);
            return *this;
        }

        AedSequence& send(std::function<void(uint8_t)> snd
                = [](uint8_t value) { std::cout << value; }) {
            snd(ESC); // enter interpreter
            for (uint8_t c : _sequence) {
                snd(c);
            }
            snd(XXX); // leave interpreter
            _sequence.clear();
            return *this;
        }

        AedSequence& write_horizontal_runs(const uint8_t* buffer, int n);

    private:
        void write_horizontal_run(const uint8_t* begin, const uint8_t* end, State state);
        void coordinate(uint16_t x, uint16_t y) {
            _sequence.push_back(((x & 0xf00) >> 4) | (y >> 8));
            _sequence.push_back(x & 0xff);
            _sequence.push_back(y & 0xff);
        }
        std::vector<uint8_t>    _sequence;
};

#endif /* LIB_AED_AEDSEQUENCE_H_ */
