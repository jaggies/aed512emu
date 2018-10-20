/*
 * coreutil.h
 *
 *  Created on: Oct 4, 2018
 *      Author: jmiller
 */

#ifndef LIB_CORE_COREUTIL_H_
#define LIB_CORE_COREUTIL_H_

#include <cstdint>

inline bool rising(uint8_t prev, uint8_t cur, uint8_t bit) {
    return !(prev & bit) && (cur & bit);
}

inline bool falling(uint8_t prev, uint8_t cur, uint8_t bit) {
    return (prev & bit) && !(cur & bit);
}

#define Number(a) (sizeof(a) / sizeof(a[0]))

#define SECS2USECS(a) ((a)*1000000)

#endif /* LIB_CORE_COREUTIL_H_ */
