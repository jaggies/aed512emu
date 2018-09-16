/*
 * io.h
 *
 *  Created on: Sep 9, 2018
 *      Author: jmiller
 */

#ifndef IO_H_
#define IO_H_

// Device control registers
#define pio0da 0x0 // PIA0 data register A
#define pio0ca 0x1 // PIA0 control register A
#define pio0db 0x2 // PIA0 data register B
#define pio0cb 0x3 // PIA0 control register B

#define pio1da 0x4 // PIA1 data register A
#define pio1ca 0x5 // PIA1 control register A
#define pio1db 0x6 // PIA1 data register B
#define pio1cb 0x7 // PIA1 control register B

#define pio2da 0x8 // PIA2 data register A
#define pio2ca 0x9 // PIA2 control register A
#define pio2db 0xa // PIA2 data register B
#define pio2cb 0xb // PIA2 control register B

#define sio0st 0xc // SIO0 status register
#define sio0da 0xd // SIO0 Tx/RX data register

#define sio1st 0xe // SIO1 status register
#define sio1da 0xf // SIO1 Tx/RX data register

#define vzoom  0x10 // vertical zoom register 0 - 15
#define hzoom  0x13 // horizontal zoom register 0 - 15

#define rmks0  0x10 // read mask0 (origin) or LL quadrant (AED767)
#define rmsk1  0x11 // read mask1 (horizontal wrap-around) or LR quadrant (AED767)
#define rmsk2  0x12 // read mask2 (vertical wrap-around) or UL quadrant (AED767)
#define rmsk3  0x13 // read mask3 (diagonal wrap-around) or UR quadrant (AED767)

#define capxl  0x14 // current address pointer, x low
#define capxh  0x15 // current address poitner, x high
#define capyl  0x16 // current address pointer, y low
#define capyh  0x17 // current address poitner, y high

#define vmnoi  0x18 // store pixel, no increment
#define vminx  0x19 // store pixel, increment or decrement x only
#define vminy  0x1a // store pixel, increment or decrement y only
#define vminxy 0x1b // store pixel, increment or decrement x and y

#define dmanoi 0x1c // DMA store pixel, no increment
#define dmainx 0x1d // DMA store pixel, increment or decrement x only
#define dmainy 0x1e // DMA store pixel, increment or decrement y only
#define dmainxy 0x1f // DMA store pixel, increment or decrement x and y

//#define notused 0x20
#define pxcntl 0x21 // DMA pixel count low. When full counter hits 0, an IRQ is triggered
#define pxcnth 0x22 // DMA pixel count high

#define wrmsk  0x23 // Write mask
#define misc0  0x24 // misc control bits [7:0] = [pixcnt_ena, yu/d, xu/d, dmadis#, b+w, wonb, yzs, z/rm]
#define hstctl 0x25 // host control [7:0] = [EXWRTDSP, BBITMODE, CMD_END, DEV_END, PDI, PDO, PSI, PCO]

#define xscrl  0x26 // X scroll
#define yscrl  0x27 // Y scroll

#define hstpl  0x28 // Host port (low byte)
#define hstph  0x29 // Host port (high byte)

#define datafl 0x2a
#define miscrd 0x2a // Misc signals: [7:0] = [DO#, DI#, RST_FLAG, BREAK, REPEAT, SHIFT, CTRL, HZ_BLANK]
#define stdvma 0x2b

#define kbdst  0x2800
#define kbsda  0x2801

// AED 1024
#define redclt  0x3800
#define grnclt  0x3900
#define bluclt  0x3a00

enum Misc0 {
    PIXCNT_EN = (1 << 7),
    Y_UD = (1 << 6), // y increment up (high) down (low)
    X_UD = (1 << 5), // x increment up (high) down (low)
    DMA_WRT_DIS = (1 << 4), // disable DMA write. TODO
    BW_EN = (1 << 3), // enable BW mode?
    W_ON_B = (1 << 2), // inverse video?
    YZS = (1 << 1), // ??? no idea
    Z_RM = 1 // ???
};

#endif /* IO_H_ */
