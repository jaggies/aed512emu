/*
 * checkpia.cpp
 *
 *  Created on: Sep 27, 2018
 *      Author: jmiller
 */

#include <cassert>
#include <iostream>
#include "68B21.h"

bool irqa_called;
bool irqb_called;

const int ca1_en_rising = M68B21::CRA0 | M68B21::CRA1; // CA1 en + rising
const int ca2_en_rising = M68B21::CRA3 | M68B21::CRA4; // CA2 en + rising
const int cb1_en_rising = M68B21::CRB0 | M68B21::CRB1; // CB1 en + rising
const int cb2_en_rising = M68B21::CRB3 | M68B21::CRB4; // CB2 en + rising
const int ca1_en_falling = M68B21::CRA0; // CA1 en + falling
const int ca2_en_falling = M68B21::CRA3; // CA2 en + falling
const int cb1_en_falling = M68B21::CRB0; // CB1 en + falling
const int cb2_en_falling = M68B21::CRB3; // CB2 en + falling

//const int ca1b1_en_falling = M68B21::CRA0;

int main(int argc, char **argv) {
    M68B21* pia = new M68B21(0, "pia",
            [](int) { ::irqa_called = true; },
            [](int) { ::irqb_called = true; });

    //
    //  Check Rising Edges
    //
    pia->reset();
    irqa_called = irqb_called = false;
    assert((pia->read(M68B21::CRA) & 0xc0) == 0); // no interrupts set yet
    assert((pia->read(M68B21::CRB) & 0xc0) == 0); // no interrupts set yet

    pia->set(M68B21::IrqStatusA, M68B21::CA1); // should not invoke irqa_callback
    pia->set(M68B21::IrqStatusA, M68B21::CA2); // should not invoke irqa_callback
    pia->set(M68B21::IrqStatusB, M68B21::CB1); // should not invoke irqa_callback
    pia->set(M68B21::IrqStatusB, M68B21::CB2); // should not invoke irqa_callback
    assert(!irqa_called && !irqb_called);
    assert((pia->read(M68B21::CRA) & 0xc0) == 0); // no interrupts set yet
    assert((pia->read(M68B21::CRB) & 0xc0) == 0); // no interrupts set yet

    // Configure CA1/CA2, CB1/CB2 to trigger on rising edge
    pia->write(M68B21::CRA, ca1_en_rising | ca2_en_rising);
    pia->write(M68B21::CRB, cb1_en_rising | cb2_en_rising);
    pia->reset(M68B21::IrqStatusA, M68B21::CA1 | M68B21::CA2);
    pia->reset(M68B21::IrqStatusB, M68B21::CB1 | M68B21::CB2);
    assert(!irqa_called && !irqb_called); // should have no effect

    // Check enabled CA1/CA2
    pia->set(M68B21::IrqStatusA, M68B21::CA1); // should invoke irqa_callback
    assert(irqa_called && !irqb_called);
    irqa_called = false;
    pia->set(M68B21::IrqStatusA, M68B21::CA1); // should not invoke irqa_callback again
    assert(!irqa_called && !irqb_called);

    pia->set(M68B21::IrqStatusA, M68B21::CA2); // should invoke irqa_callback
    assert(irqa_called && !irqb_called);
    irqa_called = false;
    pia->set(M68B21::IrqStatusA, M68B21::CA2); // should not invoke irqa_callback again
    assert(!irqa_called && !irqb_called);

    // Check CB1/CB2
    irqa_called = irqb_called = false;
    pia->set(M68B21::IrqStatusB, M68B21::CB1); // should invoke irqb_callback
    assert(!irqa_called && irqb_called);
    irqb_called = false;
    pia->set(M68B21::IrqStatusB, M68B21::CB1); // should not invoke irqb_callback again
    assert(!irqa_called && !irqb_called);

    pia->set(M68B21::IrqStatusB, M68B21::CB2); // should invoke irqb_callback
    assert(!irqa_called && irqb_called);
    irqb_called = false;
    pia->set(M68B21::IrqStatusB, M68B21::CB2); // should not invoke irqa_callback again
    assert(!irqa_called && !irqb_called);

    assert((pia->read(M68B21::CRA) & 0xc0) == (M68B21::CA1 | M68B21::CA2)); // irqA CA1 and CA2 set
    assert((pia->read(M68B21::CRB) & 0xc0) == (M68B21::CB1 | M68B21::CB2)); // irqB CB1 and CB2 set

    // Check clearing of interrupt bits CA1 and CA2
    pia->write(M68B21::CRA, M68B21::CRA2); // select PRA for read access
    pia->read(M68B21::PRA); // reading clears irq
    assert((pia->read(M68B21::CRA) & 0xc0) == 0); // no irqs set anymore
    assert((pia->read(M68B21::CRB) & 0xc0) == (M68B21::CB1 | M68B21::CB2)); // CRB should be unaffected

    // Check clearing of interrupt bits CA1 and CA2
    pia->write(M68B21::CRB, M68B21::CRB2); // select PRB for read access
    pia->read(M68B21::PRB); // reading clears irq
    assert((pia->read(M68B21::CRA) & 0xc0) == 0); // CRA should be unaffected
    assert((pia->read(M68B21::CRB) & 0xc0) == 0); // no irqs set anymore

    //
    //  Check Falling Edges
    //
    pia->reset();
    irqa_called = irqb_called = false;
    assert((pia->read(M68B21::CRA) & 0xc0) == 0); // no interrupts set yet
    assert((pia->read(M68B21::CRB) & 0xc0) == 0); // no interrupts set yet

    pia->reset(M68B21::IrqStatusA, M68B21::CA1); // should not invoke irqa_callback
    pia->reset(M68B21::IrqStatusA, M68B21::CA2); // should not invoke irqa_callback
    pia->reset(M68B21::IrqStatusB, M68B21::CB1); // should not invoke irqa_callback
    pia->reset(M68B21::IrqStatusB, M68B21::CB2); // should not invoke irqa_callback
    assert(!irqa_called && !irqb_called);
    assert((pia->read(M68B21::CRA) & 0xc0) == 0); // no interrupts set yet
    assert((pia->read(M68B21::CRB) & 0xc0) == 0); // no interrupts set yet

    // Configure CA1/CA2, CB1/CB2 to trigger on falling edge
    pia->write(M68B21::CRA, ca1_en_falling | ca2_en_falling);
    pia->write(M68B21::CRB, cb1_en_falling | cb2_en_falling);
    pia->set(M68B21::IrqStatusA, M68B21::CA1 | M68B21::CA2);
    pia->set(M68B21::IrqStatusB, M68B21::CB1 | M68B21::CB2);
    assert(!irqa_called && !irqb_called); // should have no effect

    // Check enabled CA1/CA2
    pia->reset(M68B21::IrqStatusA, M68B21::CA1); // should invoke irqa_callback
    assert(irqa_called && !irqb_called);
    irqa_called = false;
    pia->reset(M68B21::IrqStatusA, M68B21::CA1); // should not invoke irqa_callback again
    assert(!irqa_called && !irqb_called);

    pia->reset(M68B21::IrqStatusA, M68B21::CA2); // should invoke irqa_callback
    assert(irqa_called && !irqb_called);
    irqa_called = false;
    pia->reset(M68B21::IrqStatusA, M68B21::CA2); // should not invoke irqa_callback again
    assert(!irqa_called && !irqb_called);

    // Check CB1/CB2
    irqa_called = irqb_called = false;
    pia->reset(M68B21::IrqStatusB, M68B21::CB1); // should invoke irqb_callback
    assert(!irqa_called && irqb_called);
    irqb_called = false;
    pia->reset(M68B21::IrqStatusB, M68B21::CB1); // should not invoke irqb_callback again
    assert(!irqa_called && !irqb_called);

    pia->reset(M68B21::IrqStatusB, M68B21::CB2); // should invoke irqb_callback
    assert(!irqa_called && irqb_called);
    irqb_called = false;
    pia->reset(M68B21::IrqStatusB, M68B21::CB2); // should not invoke irqa_callback again
    assert(!irqa_called && !irqb_called);

    assert((pia->read(M68B21::CRA) & 0xc0) == (M68B21::CA1 | M68B21::CA2)); // irqA CA1 and CA2 set
    assert((pia->read(M68B21::CRB) & 0xc0) == (M68B21::CB1 | M68B21::CB2)); // irqB CB1 and CB2 set

    // Check clearing of interrupt bits CA1 and CA2
    pia->write(M68B21::CRA, M68B21::CRA2); // select PRA for read access
    pia->read(M68B21::PRA); // reading clears irq
    assert((pia->read(M68B21::CRA) & 0xc0) == 0); // no irqs set anymore
    assert((pia->read(M68B21::CRB) & 0xc0) == (M68B21::CB1 | M68B21::CB2)); // CRB should be unaffected

    // Check clearing of interrupt bits CA1 and CA2
    pia->write(M68B21::CRB, M68B21::CRB2); // select PRB for read access
    pia->read(M68B21::PRB); // reading clears irq
    assert((pia->read(M68B21::CRA) & 0xc0) == 0); // CRA should be unaffected
    assert((pia->read(M68B21::CRB) & 0xc0) == 0); // no irqs set anymore

    std::cerr << "*** All tests passed! ***" << std::endl;
}


