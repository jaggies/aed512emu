/*
 * mapper.cpp
 *
 *  Created on: Sep 9, 2018
 *      Author: jmiller
 */
#include <iostream>
#include "mapper.h"

std::ostream& operator<<(std::ostream& os, const Mapper& mapper) {
    os << "Device\tstart\tend\n";
    os << std::hex;
    for (Peripheral* p : mapper._peripherals) {
        if (!p) continue; // cached item
        os << p->name() << "\t" << p->start() << "\t" << p->end() - 1 << std::endl;
    }
    return os;
}
