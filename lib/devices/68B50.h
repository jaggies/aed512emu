/*
 * m68b50.h
 *
 *  Created on: Sep 10, 2018
 *      Author: jmiller
 */

#ifndef M68B50_H_
#define M68B50_H_

#include <vector>
#include "peripheral.h"

class M68B50: public Peripheral {
    public:
        enum ControlRegisterWrite { // reflected in control register
            CR0 = 1, // counter divide sel1
            CR1 = 2, // counter divide sel2
            CR2 = 4, // word select 1 (0 == 7bits, 1 == 8bits)
            CR3 = 8, // word select 2 (stop bits)
            CR4 = 16, // word select 3 (parity)
            CR5 = 32, // transmit control 1
            CR6 = 64, // transmit control 2
            CR7 = 128 // receive interrupt enable
        };

        enum ControlRegisterRead { // reflected in status register
            RDRF = 1, // receive data register full
            TDRE = 2, // transmit data register empty
            DCD  = 4, // data carrier detect
            CTS  = 8, // clear-to-send
            FE   = 16, // framing error
            OVRN = 32, // receive overrun
            PE   = 64, // parity error
            IRQ  = 128 // interrupt request
        };

        M68B50(int start, const std::string& name = "68B21")
                : Peripheral(start, 2, name), control(0), status(0), txdata(0), rxdata(0) {
            reset();
        };

        virtual ~M68B50() = default;

        // Reads peripheral register at offset
        uint8_t read(int offset) override;

        // Writes peripheral register at offset
        void write(int offset, uint8_t value) override;

        // Input to serial port. Returns true if byte can be received.
        bool receive(uint8_t data);

        // Output from serial port. Returns true if data was available
        bool transmit(uint8_t* data);

        // IRQ is set. TODO: add callback to invoke IRQ automatically
        bool irqAsserted() const { return status & IRQ; }

        // Reset to initial state
        void reset() override {
            txdata = rxdata = control = 0;
            status = TDRE | CTS | DCD; // transmit data should be empty
        }

    private:
        uint8_t control;
        uint8_t status;
        uint8_t txdata;
        uint8_t rxdata;
};

#endif /* M68B50_H_ */
