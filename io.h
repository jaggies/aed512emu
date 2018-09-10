/*
 * io.h
 *
 *  Created on: Sep 9, 2018
 *      Author: jmiller
 */

#ifndef IO_H_
#define IO_H_

// Device control registers
#define pio0da 0x1
#define pio0db 0x2
#define pio0cb 0x3
#define pio1da 0x4
#define pio1ca 0x5
#define pio1db 0x6
#define pio1cb 0x7
#define pio2da 0x8
#define pio2ca 0x9
#define pio2db 0xa
#define pio2cb 0xb
#define sio0st 0xc
#define sio0da 0xd
#define sio1st 0xe
#define sio1da 0xf
#define rmks0  0x10
#define vzoom  0x10
#define rmsk1  0x11
#define rmsk2  0x12
#define hzoom  0x13
#define rmsk3  0x13
#define capxl  0x14
#define capxh  0x15
#define capyl  0x16
#define capyh  0x17
#define vmnoi  0x18
#define vminx  0x19
#define vminy  0x1a
#define vminxy 0x1b
#define dmanoi  0x1c
#define dmainx  0x1d
#define dmainy  0x1e
#define dmainxy  0x1f
#define pxcntl  0x21
#define pxcnth  0x22
#define wrmsk  0x23
#define misc0  0x24
#define hstctl  0x25
#define xscrl  0x26
#define yscrl  0x27
#define hstpl  0x28
#define hstph  0x29
#define datafl  0x2a
#define miscrd  0x2a
#define stdvma  0x2b

#define kbdst  0x2800
#define kbsda  0x2801

// AED 1024
#define redclt  0x3800
#define grnclt  0x3900
#define bluclt  0x3a00

#endif /* IO_H_ */
