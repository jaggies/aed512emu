/*
 * util.h
 *
 *  Created on: Oct 4, 2018
 *      Author: jmiller
 */

#ifndef LIB_CORE_UTIL_H_
#define LIB_CORE_UTIL_H_

#include <cstdint>

inline bool rising(uint8_t prev, uint8_t cur, uint8_t bit) {
    return !(prev & bit) && (cur & bit);
}
inline bool falling(uint8_t prev, uint8_t cur, uint8_t bit) {
    return (prev & bit) && !(cur & bit);
}

#endif /* LIB_CORE_UTIL_H_ */
