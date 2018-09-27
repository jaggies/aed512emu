/*
 * checkpia.cpp
 *
 *  Created on: Sep 27, 2018
 *      Author: jmiller
 */

#include <cassert>
#include <iostream>
#include "68B21.h"

const int ca1b1_en_rising = M68B21::CRA0 | M68B21::CRA1; // CA1 en + rising
const int ca2b2_en_rising = M68B21::CRA3 | M68B21::CRA4; // CA2 en + rising
//const int ca1b1_en_falling = M68B21::CRA0;

int main(int argc, char **argv) {
    bool irqa_called;
    bool irqb_called;
    M68B21* pia = new M68B21(0, "pia",
            [&irqa_called](int) { irqa_called = true; },
            [&irqb_called](int) { irqb_called = true; });

    assert((pia->read(M68B21::CRA) & 0xc0) == 0); // no interrupts set yet

    // Check CA1
    pia->write(M68B21::CRA, ca1b1_en_rising);
    irqa_called = irqb_called = false;
    pia->set(M68B21::IrqStatusA, M68B21::CA1);
    assert(irqa_called);
    assert(!irqb_called);
    assert((pia->read(M68B21::CRA) & 0xc0) == M68B21::CA1); // irq CA1 set
    assert((pia->read(M68B21::CRB) & 0xc0) == 0); // CRB should be unaffected

    // Check non-enabled CA2
    pia->set(M68B21::IrqStatusA, M68B21::CA2);
    assert(irqa_called);
    assert(!irqb_called); // this one should not have been updated
    assert((pia->read(M68B21::CRA) & 0xc0) == M68B21::CA1); // irq CA2 not enable; shouldn't change
    assert((pia->read(M68B21::CRB) & 0xc0) == 0); // CRB should be unaffected

    // Check enabled CA2
    irqa_called = false;
    pia->write(M68B21::CRA, ca2b2_en_rising);
    pia->set(M68B21::IrqStatusA, M68B21::CA2);
    assert(irqa_called); // should be updated by CA2
    assert(!irqb_called);
    assert((pia->read(M68B21::CRA) & 0xc0) == (M68B21::CA1 | M68B21::CA2)); // both enabled now
    assert((pia->read(M68B21::CRB) & 0xc0) == 0); // CRB should be unaffected

    // Check clearing of interrupt bits CA1 and CA2
    pia->write(M68B21::CRA, M68B21::CRA2); // select PRA for read access
    pia->read(M68B21::PRA); // reading clears irq
    assert((pia->read(M68B21::CRA) & 0xc0) == 0); // no irqs set
    assert((pia->read(M68B21::CRB) & 0xc0) == 0); // CRB should be unaffected

    std::cerr << "*** All tests passed! ***" << std::endl;
}


