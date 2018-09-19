/*
 Public methods:
 CPU6502(CLK& clk, BUS& bus); - construct using clk and bus
 cycle() - issue one instruction and add necessary cycles to clk
 reset() - reset CPU state
 do_irq() - put CPU in IRQ
 do_nmi() - put CPU in NMI

 CLK template parameter must provide methods:
 void add_cpu_cycles(int N); - add N CPU cycles to the clock

 BUS template parameter must provide methods:
 unsigned char read(int addr);
 void read(int addr, unsigned char data);
 */

#ifndef CPU6502_H
#define CPU6502_H

#include <cstdlib>
#include <cassert>
#include <iostream>
#include <unistd.h>
#include "cpu.h"

class CPU6502 : public CPU {
    public:
        CPU6502(Reader reader, Writer writer, Counter counter)
                : CPU(reader, writer, counter), a(0), x(0), y(0), s(0xFD), p(R|I|B), pc(0),
                      pending_ex(PENDING_NONE) {
            pc = read(0xFFFC) + read(0xFFFD) * 256;
        }

        void irq() override {
            pending_ex |= PENDING_IRQ;
        }

        void nmi() override {
            // Overlapping NMIs are potentially problematic. Ensure they don't happen for now.
            assert((pending_ex & PENDING_NMI) != PENDING_NMI);
            pending_ex |= PENDING_NMI;
        }

        void reset() override {
            pending_ex |= PENDING_RESET;
        }

        void cycle() override {
            do_cycle();
        }

        int get_pc() const override { return pc; }

        // For debug/testing only
        void set_pc(int pc_) override { pc = pc_; }

        void dump(std::ostream& os) override {
            os << "pc=" << pc << " p=" << (int) p;
            os << " a=" << (int) a << " x=" << (int) x << " y=" << (int) y;
            os << " s=" << (int) s << std::endl;
        }

    private:
        static int cycles[256];

        unsigned char a, x, y, s, p; // A, X, Y, stack pointer, status register
        static const unsigned char N = 0x80;
        static const unsigned char V = 0x40;
        static const unsigned char R = 0x20; // reserved, must be '1' at all times
        static const unsigned char B = 0x10; // this bit never gets set directly
        static const unsigned char D = 0x08;
        static const unsigned char I = 0x04;
        static const unsigned char Z = 0x02;
        static const unsigned char C = 0x01;
        int pc;

        static const unsigned char PENDING_NONE = 0;
        static const unsigned char PENDING_NMI = 1;
        static const unsigned char PENDING_IRQ = 2;
        static const unsigned char PENDING_RESET = 8;
        int pending_ex = 0; // One of the above

        void stack_push(unsigned char d) {
            write(0x100 + s--, d);
        }

        unsigned char stack_pull() {
            return read(0x100 + ++s);
        }

        unsigned char read_pc_inc() {
            return read(pc++);
        }

        void flag_change(unsigned char flag, bool v) {
            if (v)
                p |= flag;
            else
                p &= ~flag;
        }

        void flag_set(unsigned char flag) {
            p |= flag;
        }

        void flag_clear(unsigned char flag) {
            p &= ~flag;
        }

        int carry() {
            return (p & C) ? 1 : 0;
        }

        bool isset(unsigned char flag) {
            return (p & flag) != 0;
        }

        void set_flags(unsigned char flags, unsigned char v) {
            if (flags & Z)
                flag_change(Z, v == 0x00);
            if (flags & N)
                flag_change(N, v & 0x80);
        }

        static bool sbc_overflow_d(unsigned char a, unsigned char b, int borrow) {
            signed char a_ = a;
            signed char b_ = b;
            signed short c = a_ - (b_ + borrow);
            return (c < 0) || (c > 99);
        }

        static bool adc_overflow_d(unsigned char a, unsigned char b, int carry) {
            signed char a_ = a;
            signed char b_ = b;
            signed short c = a_ + b_ + carry;
            return (c < 0) || (c > 99);
        }

        static bool sbc_overflow(unsigned char a, unsigned char b, int borrow) {
            signed char a_ = a;
            signed char b_ = b;
            signed short c = a_ - (b_ + borrow);
            return (c < -128) || (c > 127);
        }

        static bool adc_overflow(unsigned char a, unsigned char b, int carry) {
            signed char a_ = a;
            signed char b_ = b;
            signed short c = a_ + b_ + carry;
            return (c < -128) || (c > 127);
        }

        void do_reset() {
            s = 0xFD;
            pc = read(0xFFFC) + read(0xFFFD) * 256;
            p = R|I|B; // disable interrupts
            assert(p & R);
            pending_ex = PENDING_NONE;
            count(6); // TODO: maybe reset counter
        }

        void do_irq() {
            stack_push((pc - 1) >> 8);
            stack_push((pc - 1) & 0xFF);
            stack_push(p);
            pc = read(0xFFFE) + read(0xFFFF) * 256;
            p |= I; // disable interrupts
            pending_ex &= ~PENDING_IRQ;
            count(6);
        }

        void do_nmi() {
            stack_push((pc - 1) >> 8);
            stack_push((pc - 1) & 0xFF);
            stack_push(p);
            pc = read(0xFFFA) + read(0xFFFB) * 256;
            p |= I; // disable interrupts
            pending_ex &= ~PENDING_NMI;
            count(6);
        }

        void do_cycle() {
            unsigned char m;

            // Reset always happens, regardless of whether we're currently in an exception
            if (pending_ex & PENDING_RESET) {
                do_reset();
                return;
            } else if (pending_ex & PENDING_NMI) { // handle NMI first, ignoring state of I bit
                do_nmi();
                return;
            } else if (!(p & I) && (pending_ex & PENDING_IRQ)) {
                do_irq();
                return;
            }

            unsigned char inst = read_pc_inc();

            switch (inst) {
                case 0x00: { // BRK
                    stack_push((pc - 1) >> 8);
                    stack_push((pc - 1) & 0xFF);
                    stack_push(p | B); // | B says the Synertek 6502 reference
                    p |= I;
                    pc = read(0xFFFE) + read(0xFFFF) * 256;
                    break;
                }

                case 0xEA: { // NOP
                    break;
                }

                case 0x8A: { // TXA
                    set_flags(N | Z, a = x);
                    break;
                }

                case 0xAA: { // TAX
                    set_flags(N | Z, x = a);
                    break;
                }

                case 0xBA: { // TSX
                    set_flags(N | Z, x = s);
                    break;
                }

                case 0x9A: { // TXS
                    s = x;
                    break;
                }

                case 0xA8: { // TAY
                    set_flags(N | Z, y = a);
                    break;
                }

                case 0x98: { // TYA
                    set_flags(N | Z, a = y);
                    break;
                }

                case 0x18: { // CLC
                    flag_clear(C);
                    break;
                }

                case 0x38: { // SEC
                    flag_set(C);
                    break;
                }

                case 0xF8: { // SED
                    flag_set(D);
                    break;
                }

                case 0xD8: { // CLD
                    flag_clear(D);
                    break;
                }

                case 0x58: { // CLI
                    flag_clear(I);
                    break;
                }

                case 0x78: { // SEI
                    flag_set(I);
                    break;
                }

                case 0xB8: { // CLV
                    flag_clear(V);
                    break;
                }

                case 0xC6: { // DEC zpg
                    int zpg = read_pc_inc();
                    set_flags(N | Z, m = read(zpg) - 1);
                    write(zpg, m);
                    break;
                }

                case 0xDE: { // DEC abs, X
                    int addr = read_pc_inc() + read_pc_inc() * 256 + x;
                    set_flags(N | Z, m = read(addr) - 1);
                    write(addr, m);
                    break;
                }

                case 0xCE: { // DEC abs
                    int addr = read_pc_inc() + read_pc_inc() * 256;
                    set_flags(N | Z, m = read(addr) - 1);
                    write(addr, m);
                    break;
                }

                case 0xCA: { // DEX
                    set_flags(N | Z, x = x - 1);
                    break;
                }

                case 0xFE: { // INC abs, X
                    int addr = read_pc_inc() + read_pc_inc() * 256 + x;
                    if ((addr - x) / 256 != addr / 256)
                        count(1);
                    set_flags(N | Z, m = read(addr) + 1);
                    write(addr, m);
                    break;
                }

                case 0xEE: { // INC abs
                    int addr = read_pc_inc() + read_pc_inc() * 256;
                    set_flags(N | Z, m = read(addr) + 1);
                    write(addr, m);
                    break;
                }

                case 0xE6: { // INC zpg
                    int zpg = read_pc_inc();
                    set_flags(N | Z, m = read(zpg) + 1);
                    write(zpg, m);
                    break;
                }

                case 0xF6: { // INC zpg, X
                    int zpg = (read_pc_inc() + x) & 0xFF;
                    set_flags(N | Z, m = read(zpg) + 1);
                    write(zpg, m);
                    break;
                }

                case 0xE8: { // INX
                    set_flags(N | Z, x = x + 1);
                    break;
                }

                case 0xC8: { // INY
                    set_flags(N | Z, y = y + 1);
                    break;
                }

                case 0x10: { // BPL
                    int rel = (read_pc_inc() + 128) % 256 - 128;
                    if (!isset(N)) {
                        count(1);
                        if ((pc + rel) / 256 != pc / 256)
                            count(1);
                        pc += rel;
                    }
                    break;
                }

                case 0x50: { // BVC
                    int rel = (read_pc_inc() + 128) % 256 - 128;
                    if (!isset(V)) {
                        count(1);
                        if ((pc + rel) / 256 != pc / 256)
                            count(1);
                        pc += rel;
                    }
                    break;
                }

                case 0x70: { // BVS
                    int rel = (read_pc_inc() + 128) % 256 - 128;
                    if (isset(V)) {
                        count(1);
                        if ((pc + rel) / 256 != pc / 256)
                            count(1);
                        pc += rel;
                    }
                    break;
                }

                case 0x30: { // BMI
                    int rel = (read_pc_inc() + 128) % 256 - 128;
                    if (isset(N)) {
                        count(1);
                        if ((pc + rel) / 256 != pc / 256)
                            count(1);
                        pc += rel;
                    }
                    break;
                }

                case 0x90: { // BCC
                    int rel = (read_pc_inc() + 128) % 256 - 128;
                    if (!isset(C)) {
                        count(1);
                        if ((pc + rel) / 256 != pc / 256)
                            count(1);
                        pc += rel;
                    }
                    break;
                }

                case 0xB0: { // BCS
                    int rel = (read_pc_inc() + 128) % 256 - 128;
                    if (isset(C)) {
                        count(1);
                        if ((pc + rel) / 256 != pc / 256)
                            count(1);
                        pc += rel;
                    }
                    break;
                }

                case 0xD0: { // BNE
                    int rel = (read_pc_inc() + 128) % 256 - 128;
                    if (!isset(Z)) {
                        count(1);
                        if ((pc + rel) / 256 != pc / 256)
                            count(1);
                        pc += rel;
                    }
                    break;
                }

                case 0xF0: { // BEQ
                    int rel = (read_pc_inc() + 128) % 256 - 128;
                    if (isset(Z)) {
                        count(1);
                        if ((pc + rel) / 256 != pc / 256)
                            count(1);
                        pc += rel;
                    }
                    break;
                }

                case 0xA1: { // LDA (ind, X)
                    unsigned char zpg = (read_pc_inc() + x) & 0xFF;
                    int addr = read(zpg) + read((zpg + 1) & 0xFF) * 256;
                    set_flags(N | Z, a = read(addr));
                    break;
                }

                case 0xB5: { // LDA zpg, X
                    unsigned char zpg = read_pc_inc();
                    int addr = zpg + x;
                    set_flags(N | Z, a = read(addr & 0xFF));
                    break;
                }

                case 0xB1: { // LDA ind, Y
                    unsigned char zpg = read_pc_inc();
                    int addr = read(zpg) + read((zpg + 1) & 0xFF) * 256 + y;
                    if ((addr - y) / 256 != addr / 256)
                        count(1);
                    set_flags(N | Z, a = read(addr));
                    break;
                }

                case 0xA5: { // LDA zpg
                    unsigned char zpg = read_pc_inc();
                    set_flags(N | Z, a = read(zpg));
                    break;
                }

                case 0xDD: { // CMP abs, X
                    int addr = read_pc_inc() + read_pc_inc() * 256;
                    m = read(addr + x);
                    if ((addr + x) / 256 != addr / 256)
                        count(1);
                    flag_change(C, m <= a);
                    set_flags(N | Z, m = a - m);
                    break;
                }

                case 0xD9: { // CMP abs, Y
                    int addr = read_pc_inc() + read_pc_inc() * 256;
                    m = read(addr + y);
                    if ((addr + y) / 256 != addr / 256)
                        count(1);
                    flag_change(C, m <= a);
                    set_flags(N | Z, m = a - m);
                    break;
                }

                case 0xB9: { // LDA abs, Y
                    int addr = read_pc_inc() + read_pc_inc() * 256;
                    set_flags(N | Z, a = read(addr + y));
                    if ((addr + y) / 256 != addr / 256)
                        count(1);
                    break;
                }

                case 0xBC: { // LDY abs, X
                    int addr = read_pc_inc() + read_pc_inc() * 256;
                    set_flags(N | Z, y = read(addr + x));
                    if ((addr + x) / 256 != addr / 256)
                        count(1);
                    break;
                }

                case 0xBD: { // LDA abs, X
                    int addr = read_pc_inc() + read_pc_inc() * 256;
                    set_flags(N | Z, a = read(addr + x));
                    if ((addr + x) / 256 != addr / 256)
                        count(1);
                    break;
                }

                case 0xF5: { // SBC zpg, X
                    unsigned char zpg = (read_pc_inc() + x) & 0xFF;
                    m = read(zpg);
                    int borrow = isset(C) ? 0 : 1;
                    if (isset(D)) {
                        unsigned char bcd = a / 16 * 10 + a % 16;
                        flag_change(C, !(bcd < m + borrow));
                        flag_change(V, sbc_overflow_d(bcd, m, borrow));
                        set_flags(N | Z, bcd = bcd - (m + borrow));
                        a = bcd / 10 * 16 + bcd % 10;
                    } else {
                        flag_change(C, !(a < (m + borrow)));
                        flag_change(V, sbc_overflow(a, m, borrow));
                        set_flags(N | Z, a = a - (m + borrow));
                    }
                    break;
                }

                case 0xE5: { // SBC zpg
                    unsigned char zpg = read_pc_inc();
                    m = read(zpg);
                    int borrow = isset(C) ? 0 : 1;
                    if (isset(D)) {
                        unsigned char bcd = a / 16 * 10 + a % 16;
                        flag_change(C, !(bcd < m + borrow));
                        flag_change(V, sbc_overflow_d(bcd, m, borrow));
                        set_flags(N | Z, bcd = bcd - (m + borrow));
                        a = bcd / 10 * 16 + bcd % 10;
                    } else {
                        flag_change(C, !(a < (m + borrow)));
                        flag_change(V, sbc_overflow(a, m, borrow));
                        set_flags(N | Z, a = a - (m + borrow));
                    }
                    break;
                }

                case 0xF1: { // SBC ind, Y
                    unsigned char zpg = read_pc_inc();
                    int addr = read(zpg) + read((zpg + 1) & 0xff) * 256 + y;
                    if ((addr - y) / 256 != addr / 256)
                        count(1);
                    m = read(addr);
                    int borrow = isset(C) ? 0 : 1;
                    if (isset(D)) {
                        unsigned char bcd = a / 16 * 10 + a % 16;
                        flag_change(C, !(bcd < m + borrow));
                        flag_change(V, sbc_overflow_d(bcd, m, borrow));
                        set_flags(N | Z, bcd = bcd - (m + borrow));
                        a = bcd / 10 * 16 + bcd % 10;
                    } else {
                        flag_change(C, !(a < (m + borrow)));
                        flag_change(V, sbc_overflow(a, m, borrow));
                        set_flags(N | Z, a = a - (m + borrow));
                    }
                    break;
                }

                case 0xF9: { // SBC abs, Y
                    int addr = read_pc_inc() + read_pc_inc() * 256 + y;
                    if ((addr - y) / 256 != addr / 256)
                        count(1);
                    unsigned char m = read(addr);
                    int borrow = isset(C) ? 0 : 1;
                    if (isset(D)) {
                        unsigned char bcd = a / 16 * 10 + a % 16;
                        flag_change(C, !(bcd < m + borrow));
                        flag_change(V, sbc_overflow_d(bcd, m, borrow));
                        set_flags(N | Z, bcd = bcd - (m + borrow));
                        a = bcd / 10 * 16 + bcd % 10;
                    } else {
                        flag_change(C, !(a < (m + borrow)));
                        flag_change(V, sbc_overflow(a, m, borrow));
                        set_flags(N | Z, a = a - (m + borrow));
                    }
                    break;
                }

                case 0xFD: { // SBC abs, X
                    int addr = read_pc_inc() + read_pc_inc() * 256 + x;
                    if ((addr - x) / 256 != addr / 256)
                        count(1);
                    unsigned char m = read(addr);
                    int borrow = isset(C) ? 0 : 1;
                    if (isset(D)) {
                        unsigned char bcd = a / 16 * 10 + a % 16;
                        flag_change(C, !(bcd < m + borrow));
                        flag_change(V, sbc_overflow_d(bcd, m, borrow));
                        set_flags(N | Z, bcd = bcd - (m + borrow));
                        a = bcd / 10 * 16 + bcd % 10;
                    } else {
                        flag_change(C, !(a < (m + borrow)));
                        flag_change(V, sbc_overflow(a, m, borrow));
                        set_flags(N | Z, a = a - (m + borrow));
                    }
                    break;
                }

                case 0xED: { // SBC abs
                    int addr = read_pc_inc() + read_pc_inc() * 256;
                    unsigned char m = read(addr);
                    int borrow = isset(C) ? 0 : 1;
                    if (isset(D)) {
                        unsigned char bcd = a / 16 * 10 + a % 16;
                        flag_change(C, !(bcd < m + borrow));
                        flag_change(V, sbc_overflow_d(bcd, m, borrow));
                        set_flags(N | Z, bcd = bcd - (m + borrow));
                        a = bcd / 10 * 16 + bcd % 10;
                    } else {
                        flag_change(C, !(a < (m + borrow)));
                        flag_change(V, sbc_overflow(a, m, borrow));
                        set_flags(N | Z, a = a - (m + borrow));
                    }
                    break;
                }

                case 0xE9: { // SBC imm
                    unsigned char m = read_pc_inc();
                    int borrow = isset(C) ? 0 : 1;
                    if (isset(D)) {
                        unsigned char bcd = a / 16 * 10 + a % 16;
                        flag_change(C, !(bcd < m + borrow));
                        flag_change(V, sbc_overflow_d(bcd, m, borrow));
                        set_flags(N | Z, bcd = bcd - (m + borrow));
                        a = bcd / 10 * 16 + bcd % 10;
                    } else {
                        flag_change(C, !(a < (m + borrow)));
                        flag_change(V, sbc_overflow(a, m, borrow));
                        set_flags(N | Z, a = a - (m + borrow));
                    }
                    break;
                }

                case 0x71: { // ADC (ind), Y
                    unsigned char zpg = read_pc_inc();
                    int addr = read(zpg) + read((zpg + 1) & 0xFF) * 256 + y;
                    if ((addr - y) / 256 != addr / 256)
                        count(1);
                    m = read(addr);
                    int carry = isset(C) ? 1 : 0;
                    if (isset(D)) {
                        unsigned char bcd = a / 16 * 10 + a % 16;
                        flag_change(C, (int) (bcd + m + carry) > 99);
                        flag_change(V, adc_overflow_d(bcd, m, carry));
                        set_flags(N | Z, bcd = bcd + m + carry);
                        a = bcd / 10 * 16 + bcd % 10;
                    } else {
                        flag_change(C, (int) (a + m + carry) > 0xFF);
                        flag_change(V, adc_overflow(a, m, carry));
                        set_flags(N | Z, a = a + m + carry);
                    }
                    break;
                }

                case 0x6D: { // ADC abs
                    int addr = read_pc_inc() + read_pc_inc() * 256;
                    m = read(addr);
                    int carry = isset(C) ? 1 : 0;
                    if (isset(D)) {
                        unsigned char bcd = a / 16 * 10 + a % 16;
                        flag_change(C, (int) (bcd + m + carry) > 99);
                        flag_change(V, adc_overflow_d(bcd, m, carry));
                        set_flags(N | Z, bcd = bcd + m + carry);
                        a = bcd / 10 * 16 + bcd % 10;
                    } else {
                        flag_change(C, (int) (a + m + carry) > 0xFF);
                        flag_change(V, adc_overflow(a, m, carry));
                        set_flags(N | Z, a = a + m + carry);
                    }
                    break;
                }

                case 0x65: { // ADC
                    unsigned char zpg = read_pc_inc();
                    m = read(zpg);
                    int carry = isset(C) ? 1 : 0;
                    if (isset(D)) {
                        unsigned char bcd = a / 16 * 10 + a % 16;
                        flag_change(C, (int) (bcd + m + carry) > 99);
                        flag_change(V, adc_overflow_d(bcd, m, carry));
                        set_flags(N | Z, bcd = bcd + m + carry);
                        a = bcd / 10 * 16 + bcd % 10;
                    } else {
                        flag_change(C, (int) (a + m + carry) > 0xFF);
                        flag_change(V, adc_overflow(a, m, carry));
                        set_flags(N | Z, a = a + m + carry);
                    }
                    break;
                }

                case 0x7D: { // ADC abs, X
                    int addr = read_pc_inc() + read_pc_inc() * 256 + x;
                    if ((addr - x) / 256 != addr / 256)
                        count(1);
                    m = read(addr);
                    int carry = isset(C) ? 1 : 0;
                    if (isset(D)) {
                        unsigned char bcd = a / 16 * 10 + a % 16;
                        flag_change(C, (int) (bcd + m + carry) > 99);
                        flag_change(V, adc_overflow_d(bcd, m, carry));
                        set_flags(N | Z, bcd = bcd + m + carry);
                        a = bcd / 10 * 16 + bcd % 10;
                    } else {
                        flag_change(C, (int) (a + m + carry) > 0xFF);
                        flag_change(V, adc_overflow(a, m, carry));
                        set_flags(N | Z, a = a + m + carry);
                    }
                    break;
                }

                case 0x79: { // ADC abs, Y
                    int addr = read_pc_inc() + read_pc_inc() * 256 + y;
                    if ((addr - y) / 256 != addr / 256)
                        count(1);
                    m = read(addr);
                    int carry = isset(C) ? 1 : 0;
                    if (isset(D)) {
                        unsigned char bcd = a / 16 * 10 + a % 16;
                        flag_change(C, (int) (bcd + m + carry) > 99);
                        flag_change(V, adc_overflow_d(bcd, m, carry));
                        set_flags(N | Z, bcd = bcd + m + carry);
                        a = bcd / 10 * 16 + bcd % 10;
                    } else {
                        flag_change(C, (int) (a + m + carry) > 0xFF);
                        flag_change(V, adc_overflow(a, m, carry));
                        set_flags(N | Z, a = a + m + carry);
                    }
                    break;
                }

                case 0x69: { // ADC
                    m = read_pc_inc();
                    int carry = isset(C) ? 1 : 0;
                    if (isset(D)) {
                        unsigned char bcd = a / 16 * 10 + a % 16;
                        flag_change(C, (int) (bcd + m + carry) > 99);
                        flag_change(V, adc_overflow_d(bcd, m, carry));
                        set_flags(N | Z, bcd = bcd + m + carry);
                        a = bcd / 10 * 16 + bcd % 10;
                    } else {
                        flag_change(C, (int) (a + m + carry) > 0xFF);
                        flag_change(V, adc_overflow(a, m, carry));
                        set_flags(N | Z, a = a + m + carry);
                    }
                    break;
                }

                case 0x0E: { // ASL abs
                    int addr = read_pc_inc() + read_pc_inc() * 256;
                    m = read(addr);
                    flag_change(C, m & 0x80);
                    set_flags(N | Z, m = m << 1);
                    write(addr, m);
                    break;
                }

                case 0x06: { // ASL
                    unsigned char zpg = read_pc_inc();
                    m = read(zpg);
                    flag_change(C, m & 0x80);
                    set_flags(N | Z, m = m << 1);
                    write(zpg, m);
                    break;
                }

                case 0x16: { // ASL
                    unsigned char zpg = read_pc_inc();
                    m = read((zpg + x) & 0xFF);
                    flag_change(C, m & 0x80);
                    set_flags(N | Z, m = m << 1);
                    write((zpg + x) & 0xFF, m);
                    break;
                }

                case 0x0A: { // ASL
                    flag_change(C, a & 0x80);
                    set_flags(N | Z, a = a << 1);
                    break;
                }

                case 0x5E: { // LSR abs, X
                    int addr = read_pc_inc() + read_pc_inc() * 256;
                    m = read(addr + x);
                    flag_change(C, m & 0x01);
                    set_flags(N | Z, m = m >> 1);
                    write(addr + x, m);
                    break;
                }

                case 0x46: { // LSR
                    unsigned char zpg = read_pc_inc();
                    m = read(zpg);
                    flag_change(C, m & 0x01);
                    set_flags(N | Z, m = m >> 1);
                    write(zpg, m);
                    break;
                }

                case 0x56: { // LSR zpg, X
                    unsigned char zpg = read_pc_inc() + x;
                    m = read(zpg & 0xFF);
                    flag_change(C, m & 0x01);
                    set_flags(N | Z, m = m >> 1);
                    write(zpg, m);
                    break;
                }

                case 0x4E: { // LSR
                    int addr = read_pc_inc() + read_pc_inc() * 256;
                    m = read(addr);
                    flag_change(C, m & 0x01);
                    set_flags(N | Z, m = m >> 1);
                    write(addr, m);
                    break;
                }

                case 0x4A: { // LSR
                    flag_change(C, a & 0x01);
                    set_flags(N | Z, a = a >> 1);
                    break;
                }

                case 0x68: { // PLA
                    set_flags(N | Z, a = stack_pull());
                    break;
                }

                case 0x48: { // PHA
                    stack_push(a);
                    break;
                }

                case 0x01: { // ORA (ind, X)
                    unsigned char zpg = (read_pc_inc() + x) & 0xFF;
                    int addr = read(zpg) + read((zpg + 1) & 0xFF) * 256;
                    m = read(addr);
                    set_flags(N | Z, a = a | m);
                    break;
                }

                case 0x15: { // ORA zpg, X
                    int zpg = (read_pc_inc() + x) & 0xFF;
                    m = read(zpg);
                    set_flags(N | Z, a = a | m);
                    break;
                }

                case 0x0D: { // ORA abs
                    int addr = read_pc_inc() + read_pc_inc() * 256;
                    m = read(addr);
                    set_flags(N | Z, a = a | m);
                    break;
                }

                case 0x19: { // ORA abs, Y
                    int addr = read_pc_inc() + read_pc_inc() * 256;
                    m = read(addr + y);
                    if ((addr + y) / 256 != addr / 256)
                        count(1);
                    set_flags(N | Z, a = a | m);
                    break;
                }

                case 0x1D: { // ORA abs, X
                    int addr = read_pc_inc() + read_pc_inc() * 256;
                    m = read(addr + x);
                    if ((addr + x) / 256 != addr / 256)
                        count(1);
                    set_flags(N | Z, a = a | m);
                    break;
                }

                case 0x11: { // ORA (ind), Y
                    unsigned char zpg = read_pc_inc();
                    int addr = read(zpg) + read((zpg + 1) & 0xFF) * 256 + y;
                    if ((addr - y) / 256 != addr / 256)
                        count(1);
                    m = read(addr);
                    set_flags(N | Z, a = a | m);
                    break;
                }

                case 0x05: { // ORA zpg
                    unsigned char zpg = read_pc_inc();
                    m = read(zpg);
                    set_flags(N | Z, a = a | m);
                    break;
                }

                case 0x09: { // ORA imm
                    unsigned char imm = read_pc_inc();
                    set_flags(N | Z, a = a | imm);
                    break;
                }

                case 0x35: { // AND zpg, X
                    int zpg = (read_pc_inc() + x) & 0xFF;
                    set_flags(N | Z, a = a & read(zpg));
                    break;
                }

                case 0x31: { // AND (ind), y
                    unsigned char zpg = read_pc_inc();
                    int addr = read(zpg) + read((zpg + 1) & 0xFF) * 256 + y;
                    if ((addr - y) / 256 != addr / 256)
                        count(1);
                    set_flags(N | Z, a = a & read(addr));
                    break;
                }

                case 0x3D: { // AND abs, x
                    int addr = read_pc_inc() + read_pc_inc() * 256;
                    set_flags(N | Z, a = a & read(addr + x));
                    if ((addr + x) / 256 != addr / 256)
                        count(1);
                    break;
                }

                case 0x39: { // AND abs, y
                    int addr = read_pc_inc() + read_pc_inc() * 256;
                    set_flags(N | Z, a = a & read(addr + y));
                    if ((addr + y) / 256 != addr / 256)
                        count(1);
                    break;
                }

                case 0x2D: { // AND abs
                    int addr = read_pc_inc() + read_pc_inc() * 256;
                    set_flags(N | Z, a = a & read(addr));
                    break;
                }

                case 0x25: { // AND zpg
                    unsigned char zpg = read_pc_inc();
                    set_flags(N | Z, a = a & read(zpg));
                    break;
                }

                case 0x29: { // AND imm
                    unsigned char imm = read_pc_inc();
                    set_flags(N | Z, a = a & imm);
                    break;
                }

                case 0x88: { // DEY
                    set_flags(N | Z, y = y - 1);
                    break;
                }

                case 0x7E: { // ROR abs, X
                    int addr = read_pc_inc() + read_pc_inc() * 256;
                    m = read(addr + x);
                    bool c = isset(C);
                    flag_change(C, m & 0x80);
                    set_flags(N | Z, m = (c ? 0x80 : 0x00) | (m >> 1));
                    write(addr + x, m);
                    break;
                }

                case 0x36: { // ROL zpg,X
                    unsigned char zpg = (read_pc_inc() + x) & 0xFF;
                    m = read(zpg);
                    bool c = isset(C);
                    flag_change(C, m & 0x01);
                    set_flags(N | Z, m = (c ? 0x01 : 0x00) | (m << 1));
                    write(zpg, m);
                    break;
                }

                case 0x3E: { // ROL abs, X
                    int addr = read_pc_inc() + read_pc_inc() * 256;
                    m = read(addr + x);
                    bool c = isset(C);
                    flag_change(C, m & 0x80);
                    set_flags(N | Z, m = (c ? 0x01 : 0x00) | (m << 1));
                    write(addr + x, m);
                    break;
                }

                case 0x2A: { // ROL
                    bool c = isset(C);
                    flag_change(C, a & 0x80);
                    set_flags(N | Z, a = (c ? 0x01 : 0x00) | (a << 1));
                    break;
                }

                case 0x6A: { // ROR
                    bool c = isset(C);
                    flag_change(C, a & 0x01);
                    set_flags(N | Z, a = (c ? 0x80 : 0x00) | (a >> 1));
                    break;
                }

                case 0x6E: { // ROR abs
                    int addr = read_pc_inc() + read_pc_inc() * 256;
                    m = read(addr);
                    bool c = isset(C);
                    flag_change(C, m & 0x01);
                    set_flags(N | Z, m = (c ? 0x80 : 0x00) | (m >> 1));
                    write(addr, m);
                    break;
                }

                case 0x66: { // ROR
                    unsigned char zpg = read_pc_inc();
                    m = read(zpg);
                    bool c = isset(C);
                    flag_change(C, m & 0x01);
                    set_flags(N | Z, m = (c ? 0x80 : 0x00) | (m >> 1));
                    write(zpg, m);
                    break;
                }

                case 0x76: { // ROR
                    unsigned char zpg = (read_pc_inc() + x) & 0xFF;
                    m = read(zpg);
                    bool c = isset(C);
                    flag_change(C, m & 0x01);
                    set_flags(N | Z, m = (c ? 0x80 : 0x00) | (m >> 1));
                    write(zpg, m);
                    break;
                }

                case 0x2E: { // ROL abs
                    int addr = read_pc_inc() + read_pc_inc() * 256;
                    m = read(addr);
                    bool c = isset(C);
                    flag_change(C, m & 0x80);
                    set_flags(N | Z, m = (c ? 0x01 : 0x00) | (m << 1));
                    write(addr, m);
                    break;
                }

                case 0x26: { // ROL
                    unsigned char zpg = read_pc_inc();
                    bool c = isset(C);
                    m = read(zpg);
                    flag_change(C, m & 0x80);
                    set_flags(N | Z, m = (c ? 0x01 : 0x00) | (m << 1));
                    write(zpg, m);
                    break;
                }

                case 0x4C: { // JMP
                    int addr = read_pc_inc() + read_pc_inc() * 256;
                    pc = addr;
                    break;
                }

                case 0x6C: { // JMP
                    int addr = read_pc_inc() + read_pc_inc() * 256;
                    unsigned char addrl = read(addr);
                    unsigned char addrh = read(addr + 1);
                    addr = addrl + addrh * 256;
                    pc = addr;
                    break;
                }

                case 0x9D: { // STA abs, x
                    int addr = read_pc_inc() + read_pc_inc() * 256;
                    write(addr + x, a);
                    break;
                }

                case 0x99: { // STA
                    int addr = read_pc_inc() + read_pc_inc() * 256;
                    write(addr + y, a);
                    break;
                }

                case 0x91: { // STA (ind), Y
                    unsigned char zpg = read_pc_inc();
                    int addr = read(zpg) + read((zpg + 1) & 0xFF) * 256 + y;
                    write(addr, a);
                    break;
                }

                case 0x81: { // STA (ind, X)
                    unsigned char zpg = (read_pc_inc() + x) & 0xFF;
                    int addr = read(zpg) + read((zpg + 1) & 0xFF) * 256;
                    write(addr, a);
                    break;
                }

                case 0x8D: { // STA
                    int addr = read_pc_inc() + read_pc_inc() * 256;
                    write(addr, a);
                    break;
                }

                case 0x08: { // PHP
                    stack_push(p);
                    break;
                }

                case 0x28: { // PLP
                    p = R | B | stack_pull();
                    break;
                }

                case 0x24: { // BIT
                    unsigned char zpg = read_pc_inc();
                    m = read(zpg);
                    flag_change(Z, (a & m) == 0);
                    flag_change(N, m & 0x80);
                    flag_change(V, m & 0x40);
                    break;
                }

                case 0x2C: { // BIT
                    int addr = read_pc_inc() + read_pc_inc() * 256;
                    m = read(addr);
                    flag_change(Z, (a & m) == 0);
                    flag_change(N, m & 0x80);
                    flag_change(V, m & 0x40);
                    break;
                }

                case 0xB4: { // LDY
                    unsigned char zpg = read_pc_inc();
                    set_flags(N | Z, y = read((zpg + x) & 0xFF));
                    break;
                }

                case 0xAE: { // LDX abs
                    int addr = read_pc_inc() + read_pc_inc() * 256;
                    set_flags(N | Z, x = read(addr));
                    break;
                }

                case 0xBE: { // LDX
                    int addr = read_pc_inc() + read_pc_inc() * 256 + y;
                    if ((addr - y) / 256 != addr / 256)
                        count(1);
                    set_flags(N | Z, x = read(addr));
                    break;
                }

                case 0xA6: { // LDX
                    unsigned char zpg = read_pc_inc();
                    set_flags(N | Z, x = read(zpg));
                    break;
                }

                case 0xA4: { // LDY
                    unsigned char zpg = read_pc_inc();
                    set_flags(N | Z, y = read(zpg));
                    break;
                }

                case 0xAC: { // LDY
                    int addr = read_pc_inc() + read_pc_inc() * 256;
                    set_flags(N | Z, y = read(addr));
                    break;
                }

                case 0xA2: { // LDX
                    unsigned char imm = read_pc_inc();
                    set_flags(N | Z, x = imm);
                    break;
                }

                case 0xA0: { // LDY
                    unsigned char imm = read_pc_inc();
                    set_flags(N | Z, y = imm);
                    break;
                }

                case 0xA9: { // LDA
                    unsigned char imm = read_pc_inc();
                    set_flags(N | Z, a = imm);
                    break;
                }

                case 0xAD: { // LDA
                    int addr = read_pc_inc() + read_pc_inc() * 256;
                    set_flags(N | Z, a = read(addr));
                    break;
                }

                case 0xCC: { // CPY abs
                    int addr = read_pc_inc() + read_pc_inc() * 256;
                    m = read(addr);
                    flag_change(C, m <= y);
                    set_flags(N | Z, m = y - m);
                    break;
                }

                case 0xEC: { // CPX abs
                    int addr = read_pc_inc() + read_pc_inc() * 256;
                    m = read(addr);
                    flag_change(C, m <= x);
                    set_flags(N | Z, m = x - m);
                    break;
                }

                case 0xE0: { // CPX
                    unsigned char imm = read_pc_inc();
                    flag_change(C, imm <= x);
                    set_flags(N | Z, imm = x - imm);
                    break;
                }

                case 0xC0: { // CPY
                    unsigned char imm = read_pc_inc();
                    flag_change(C, imm <= y);
                    set_flags(N | Z, imm = y - imm);
                    break;
                }

                case 0x55: { // EOR zpg, X
                    unsigned char zpg = read_pc_inc() + x;
                    m = read(zpg & 0xFF);
                    set_flags(N | Z, a = a ^ m);
                    break;
                }

                case 0x41: { // EOR (ind, X)
                    unsigned char zpg = (read_pc_inc() + x) & 0xFF;
                    int addr = read(zpg) + read((zpg + 1) & 0xFF) * 256;
                    m = read(addr);
                    set_flags(N | Z, a = a ^ m);
                    break;
                }

                case 0x4D: { // EOR abs
                    int addr = read_pc_inc() + read_pc_inc() * 256;
                    m = read(addr);
                    set_flags(N | Z, a = a ^ m);
                    break;
                }

                case 0x5D: { // EOR abs, X
                    int addr = read_pc_inc() + read_pc_inc() * 256;
                    m = read(addr + x);
                    if ((addr + x) / 256 != addr / 256)
                        count(1);
                    set_flags(N | Z, a = a ^ m);
                    break;
                }

                case 0x59: { // EOR abs, Y
                    int addr = read_pc_inc() + read_pc_inc() * 256;
                    m = read(addr + y);
                    if ((addr + y) / 256 != addr / 256)
                        count(1);
                    set_flags(N | Z, a = a ^ m);
                    break;
                }

                case 0x45: { // EOR
                    unsigned char zpg = read_pc_inc();
                    set_flags(N | Z, a = a ^ read(zpg));
                    break;
                }

                case 0x49: { // EOR
                    unsigned char imm = read_pc_inc();
                    set_flags(N | Z, a = a ^ imm);
                    break;
                }

                case 0x51: { // EOR
                    unsigned char zpg = read_pc_inc();
                    int addr = read(zpg) + read((zpg + 1) & 0xFF) * 256 + y;
                    if ((addr - y) / 256 != addr / 256)
                        count(1);
                    m = read(addr);
                    set_flags(N | Z, a = a ^ m);
                    break;
                }

                case 0xD1: { // CMP
                    unsigned char zpg = read_pc_inc();
                    int addr = read(zpg) + read((zpg + 1) & 0xFF) * 256 + y;
                    if ((addr - y) / 256 != addr / 256)
                        count(1);
                    m = read(addr);
                    flag_change(C, m <= a);
                    set_flags(N | Z, m = a - m);
                    break;
                }

                case 0xC5: { // CMP
                    unsigned char zpg = read_pc_inc();
                    m = read(zpg);
                    flag_change(C, m <= a);
                    set_flags(N | Z, m = a - m);
                    break;
                }

                case 0xCD: { // CMP
                    int addr = read_pc_inc() + read_pc_inc() * 256;
                    m = read(addr);
                    flag_change(C, m <= a);
                    set_flags(N | Z, m = a - m);
                    break;
                }

                case 0xC9: { // CMP
                    unsigned char imm = read_pc_inc();
                    flag_change(C, imm <= a);
                    set_flags(N | Z, imm = a - imm);
                    break;
                }

                case 0xD5: { // CMP
                    unsigned char zpg = read_pc_inc() + x;
                    m = read(zpg & 0xFF);
                    flag_change(C, m <= a);
                    set_flags(N | Z, m = a - m);
                    break;
                }

                case 0xE4: { // CPX
                    unsigned char zpg = read_pc_inc();
                    m = read(zpg);
                    flag_change(C, m <= x);
                    set_flags(N | Z, m = x - m);
                    break;
                }

                case 0xC4: { // CPY
                    unsigned char zpg = read_pc_inc();
                    m = read(zpg);
                    flag_change(C, m <= y);
                    set_flags(N | Z, m = y - m);
                    break;
                }

                case 0x85: { // STA
                    unsigned char zpg = read_pc_inc();
                    write(zpg, a);
                    break;
                }

                case 0x40: { // RTI
                    p = R | stack_pull();
                    unsigned char pcl = stack_pull();
                    unsigned char pch = stack_pull();
                    pc = pcl + pch * 256 + 1;
                    break;
                }

                case 0x60: { // RTS
                    unsigned char pcl = stack_pull();
                    unsigned char pch = stack_pull();
                    pc = pcl + pch * 256 + 1;
                    break;
                }

                case 0x95: { // STA
                    unsigned char zpg = read_pc_inc();
                    write((zpg + x) & 0xFF, a);
                    break;
                }

                case 0x94: { // STY
                    unsigned char zpg = read_pc_inc();
                    write((zpg + x) & 0xFF, y);
                    break;
                }

                case 0x8E: { // STX abs
                    int addr = read_pc_inc() + read_pc_inc() * 256;
                    write(addr, x);
                    break;
                }

                case 0x86: { // STX
                    unsigned char zpg = read_pc_inc();
                    write(zpg, x);
                    break;
                }

                case 0x84: { // STY
                    unsigned char zpg = read_pc_inc();
                    write(zpg, y);
                    break;
                }

                case 0x8C: { // STY
                    int addr = read_pc_inc() + read_pc_inc() * 256;
                    write(addr, y);
                    break;
                }

                case 0x20: { // JSR
                    stack_push((pc + 1) >> 8);
                    stack_push((pc + 1) & 0xFF);
                    int addr = read_pc_inc() + read_pc_inc() * 256;
                    pc = addr;
                    break;
                }

                    // 65C02 instructions

                case 0x80: { // BRA imm, 65C02
                    int rel = (read_pc_inc() + 128) % 256 - 128;
                    if ((pc + rel) / 256 != pc / 256)
                        count(1);
                    pc += rel;
                    break;
                }

                case 0x64: { // STZ zpg, 65C02
                    unsigned char zpg = read_pc_inc();
                    write(zpg, 0);
                    break;
                }

                case 0x9C: { // STZ abs, 65C02
                    int addr = read_pc_inc() + read_pc_inc() * 256;
                    write(addr, 0x0);
                    break;
                }

                case 0xDA: { // PHX, 65C02
                    stack_push(x);
                    break;
                }

                case 0xB2: { // LDA (zpg), 65C02
                    unsigned char zpg = read_pc_inc();
                    int addr = read(zpg) + read((zpg + 1) & 0xFF) * 256;
                    set_flags(N | Z, a = read(addr));
                    break;
                }

                case 0x92: { // STA (zpg), 65C02
                    unsigned char zpg = read_pc_inc();
                    int addr = read(zpg) + read((zpg + 1) & 0xFF) * 256;
                    write(addr, a);
                    break;
                }

                case 0x72: { // ADC (zpg), 65C02
                    unsigned char zpg = read_pc_inc();
                    int addr = read(zpg) + read((zpg + 1) & 0xFF) * 256;

                    m = read(addr);
                    int carry = isset(C) ? 1 : 0;
                    if (isset(D)) {
                        unsigned char bcd = a / 16 * 10 + a % 16;
                        flag_change(C, (int) (bcd + m + carry) > 99);
                        flag_change(V, adc_overflow_d(bcd, m, carry));
                        set_flags(N | Z, bcd = bcd + m + carry);
                        a = bcd / 10 * 16 + bcd % 10;
                    } else {
                        flag_change(C, (int) (a + m + carry) > 0xFF);
                        flag_change(V, adc_overflow(a, m, carry));
                        set_flags(N | Z, a = a + m + carry);
                    }
                    break;
                }

                case 0x3A: { // DEC, 65C02
                    set_flags(N | Z, a = a - 1);
                    break;
                }

                case 0x1A: { // INC, 65C02
                    set_flags(N | Z, a = a + 1);
                    break;
                }

                case 0x12: { // ORA (ind), 65C02
                    unsigned char zpg = read_pc_inc();
                    int addr = read(zpg) + read((zpg + 1) & 0xFF) * 256;
                    m = read(addr);
                    set_flags(N | Z, a = a | m);
                    break;
                }

                case 0xD2: { // CMP (zpg), 65C02 instruction
                    unsigned char zpg = read_pc_inc();
                    int addr = read(zpg) + read((zpg + 1) & 0xFF) * 256;
                    m = read(addr);
                    flag_change(C, m <= a);
                    set_flags(N | Z, m = a - m);
                    break;
                }

                default:
                    printf("unhandled instruction %02X at %04X\n", inst, pc - 1);
                    fflush (stdout);
                    while(1) sleep(1);
                    exit(1);
            }
            assert(cycles[inst] > 0);
            count(cycles[inst]);
        }
};

int CPU6502::cycles[256] = {
/* 0x0- */7, 6, -1, -1, -1, 3, 5, -1, 3, 2, 2, -1, -1, 4, 6, -1,
/* 0x1- */2, 5, 5, -1, -1, 4, 6, -1, 2, 4, 2, -1, -1, 4, 7, -1,
/* 0x2- */6, 6, -1, -1, 3, 3, 5, -1, 4, 2, 2, -1, 4, 4, 6, -1,
/* 0x3- */2, 5, -1, -1, -1, 4, 6, -1, 2, 4, 2, -1, -1, 4, 7, -1,
/* 0x4- */6, 6, -1, -1, -1, 3, 5, -1, 3, 2, 2, -1, 3, 4, 6, -1,
/* 0x5- */2, 5, -1, -1, -1, 4, 6, -1, 2, 4, -1, -1, -1, 4, 7, -1,
/* 0x6- */6, 6, -1, -1, 3, 3, 5, -1, 4, 2, 2, -1, 5, 4, 6, -1,
/* 0x7- */2, 5, 5, -1, -1, 4, 6, -1, 2, 4, -1, -1, -1, 4, 7, -1,
/* 0x8- */2, 6, -1, -1, 3, 3, 3, -1, 2, -1, 2, -1, 4, 4, 4, -1,
/* 0x9- */2, 6, 5, -1, 4, 4, 4, -1, 2, 5, 2, -1, 4, 5, -1, -1,
/* 0xA- */2, 6, 2, -1, 3, 3, 3, -1, 2, 2, 2, -1, 4, 4, 4, -1,
/* 0xB- */2, 5, 5, -1, 4, 4, 4, -1, 2, 4, 2, -1, 4, 4, 4, -1,
/* 0xC- */2, 6, -1, -1, 3, 3, 5, -1, 2, 2, 2, -1, 4, 4, 3, -1,
/* 0xD- */2, 5, 5, -1, -1, 4, 6, -1, 2, 4, 3, -1, -1, 4, 7, -1,
/* 0xE- */2, 6, -1, -1, 3, 3, 5, -1, 2, 2, 2, -1, 4, 4, 6, -1,
/* 0xF- */2, 5, -1, -1, -1, 4, 6, -1, 2, 4, -1, -1, -1, 4, 7, -1, };

#endif // CPU6502_H

