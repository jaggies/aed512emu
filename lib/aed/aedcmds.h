/*
 * aedcmds.h
 *
 *  Created on: Sep 24, 2018
 *      Author: jmiller
 */

#ifndef LIB_AED_AEDCMDS_H_
#define LIB_AED_AEDCMDS_H_

#define AAV   "+$"  // anti-alias vector (aav <status>)
#define AED   "+."  // show software version #
#define BFL   'B'   // boundary fill (bfl <color>)
#define BLG   "+%"  // turn grid on and off (blg <0/1>)
#define BSO   'g'   // set horz and vert orig regs (bso <x><y>)
#define CAI   "++"  // copy area of interest (cai <x><y>)
#define CHR   "+-"  // set character size chr <font>
#define COP   ')'   // make copy (cop <dev><#copies>)
#define DAI   'r'   // define area of interest (dai <x><y>)
#define DCA   'p'   // erase cursor and redraw at new x,y (dca <x><y>)
#define DCL   'O'   // draw circle (dcl <radius>)
#define DFC   'n'   // draw fat circle (dfc <radius>)
#define DFP   '!'   // draw filled polygon (dfp<wc><cnt><x1><y1>..<xn><yn>)
#define DFR   'o'   // draw filled rectangle (dfr <x><y>)
#define DJC   'd'   // disable joystick cursor
#define DMV   'm'   // draw mult vectors (dmv <dx1><dy1><dx2><dy2>...<0><0>)
#define DPA   '&'   // disable panning
#define DPK   'N'   // define pgm ftn key (dpk <key><chr1>..<chr8>)
#define DRL   '='   // set led display to VAL (old kbd only) (drl <byte>)
#define DSF   '7'   // define special font (dsf <code><hsiz><vsiz><mask><byte1>..<byten><0>)
#define DSK   '%'   // define soft key (dsk <key><cnt><chr1>..<chrn>)
#define DSP   "+!"  // define stipple pattern used for filling rectangles (dsp <npat><pat1>..<pat8>)
#define DTM   '2'   // define tablet mapping (dtm <xorg><yorg><xscal><yscal>)
#define DVA   'A'   // draw vec absolute (dva <x><y>)
#define DVR   'l'   // draw vec relative (dvr <dx><dy>)
#define ECU   '5'   // erase cursor unconditionaly
#define EJC   'U'   // enable cursor pos via joystick
#define ELP   '\"'  // draw ellipse (elp <x><y><const>)
#define EPA   'h'   // enable panning
#define ERS   '~'   // erase entire memory
#define ESC   27    // ignored as function code, begins all commmands
#define ESF   '9'   // erase special font (esf <hsiz><vsiz><dx><dy>)
#define ETC   '3'   // enable tablet cursor (etc <idchr>)
#define ETP   "+'"  // enable tablet polling
#define FFD   12    // form feed
#define FRR   ','   // filled rectange relative (frr <dx><dy>)
#define GFL   '$'   // general fill (gfl <mask><bcolor>)
#define GS    29    // enter TEK graphics output mode
#define HOM   '_'   // home
#define HSR   'w'   // horz scroll relative (hsr <dx1>)
#define IFL   'I'   // interior fill
#define JUS   ';'   // jump user subroutine (jus <addr>)
#define LAT   "+&"  // ld clr tbl with anti aliasing ramp (lat <stadr><v1>..<v16>)
#define LMR   ':'   // load microprocessor ram (lmr <adr><cnt><val1>..<valn>)
#define MAR   "+*"  // set left and right margins (mar <left><right>)
#define MOV   'Q'   // set access position absolute (mov <x><y>)
#define MVR   'i'   // set access position relative (mvr <dx><dy>)
#define OFL   'V'   // overlay fill
#define OPT   '('   // set programmable options (opt <option><val>)
#define PEK   "+0"  // peek into 6502 memory (pek <adr>)
#define POK   "+1"  // poke into 6502 memory pok <adr><val>
#define RCP   'j'   // read cursor position
#define RCT   "+#"  // read color table (rct <stadr><cnt>)
#define RDA   '\\'  // read direct from area of interest
#define RHO   'y'   // read horz origin
#define RHR   'a'   // read  horz runs
#define RHS   't'   // read  horz scan
#define RJP   'q'   // read joystick position
#define ROT   "+,"  // rotate area of interest (rot <count>)
#define RPX   'Y'   // read pixel
#define RRD   '@'   // read raster direct
#define RST   '0'   // full terminal reset
#define RTP   '!'   // read tablet postion
#define RVO   'z'   // read vert origin
#define RZR   "+/"  // read zoom registers
#define SAC   '{'   // set alphanumeric cursor color (sec <color>)
#define SAP   '^'   // set alphanumeric parameters (sap <size><font><hspace><vspace><link>)
#define SAR   '#'   // set auto roam (not impl) (sar <xysign><xrate><yrate>)
#define SBC   '['   // set current color (background) (sbc <color>)
#define SBL   '4'   // blink all pixels with specified color (sbl <clr><r><g><b><on><off>)
#define SBR   'b'   // set baud rate sbr <main><aux>
#define SCC   'c'   // set cursor colors (scc <clr1><clr2><blinktime>)
#define SCD   '*'   // start cmd DMA (see XCD to terminate)
#define SCP   ']'   // set cursor parameters (scp <shape><constraint><plane(s)>)
#define SCR   '>'   // send carriage return
#define SCS   '`'   // set control status (scs <byte>)
#define SCT   'K'   // set color table (sct <adr><n><r><g><b>)
#define SDA   ^[    // stop dir access read or write
#define SEC   'C'   // set current color (foreground) (sec <color>)
#define SEN   'G'   // set encoding (sen <ftype><otype><rtype><ctype><ptype>)
#define SHO   'f'   // set horiz orig reg (sho <val>)
#define SIF   'H'   // select intf for returned options (sif <type>)
#define SKS   '6'   // send keystroke (parallel only)
#define SLS   '1'   // set line style (sls <pattern><scale>)
#define SPF   '\"'  // select stipple pattern (spf <pattern#>)
#define SRM   'M'   // set read mask (srm <msk0><msk1><msk2><msk3>)
#define SSE   '}'   // set stack end (sse <adr>)
#define STD   '\''  // set turnaround delay (std <delay>)
#define STP   "+("  // set tablet parameters (stp <byte>)
#define STW   "+)"  // set tektronix window (stw <0/1>)
#define SUB   26    // enter TEK graphics input (GIN) mode
#define SUC   '?'   // set up counters for DVMA suc <byte>
#define SUP   '-'   // ena/dsb superoam (512 only) (sup <0/1>)
#define SVO   'e'   // set vert orig (svo <val>)
#define SWM   'L'   // set video memory write mask (swm <val>)
#define SZR   'E'   // set zoom registers (szr <xzoom><yzoom>)
#define VSR   'x'   // vert scroll relative (vsr <dy1>)
#define WDA   '.'   // write direct into area of interest
#define WHC   'u'   // write horz scan (non-AOI) (whc <count><byte1>..<byten>)
#define WHR   '\\'  // write horz runs (whr <count><color><0>)
#define WHS   'X'   // write horz scan (whs <byte1>...<byten>)
#define WHU   's'   // write horz runs (alt) (whu <length><color> /  whu <255><n><color>
                    // whu <length><color><255><n><color> intermixed)
#define WIP   'v'   // write incr plotter mode (wip <n16><byte>)
#define WMP   'k'   // write mult isolated pixels (wmp <dx><dy>)
#define WPX   'T'   // write pixel (wpx <color>)
#define WRD   'F'   // write raster direct (wrd <count>)
#define WSF   '8'   // write special font wsf <code><dx><dy><0>
#define XCD   '<'   // exit  cmd DMA
#define XTD   '+'   // extended command mode
#define XXX   13    // exit graphics interpreter

#endif /* LIB_AED_AEDCMDS_H_ */
