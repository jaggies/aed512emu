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
        // Function to send to host. Returns false if data cannot be received by host.
        // If this happens, the host must manually call transmit() to retrieve bytes.
        typedef std::function<bool(uint8_t)> SendToHost;
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
            IRQW = 128 // interrupt request waiting
        };

        M68B50(int start, const std::string& name = "68B21", IRQ irq = nullptr, SendToHost send = nullptr)
                : Peripheral(start, 2, name), _control(0), _status(TDRE | CTS | DCD), _txdata(0), _rxdata(0),
                  _irq(irq), _send(send) {
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

        // Reset to initial state
        void reset() override {
            _txdata = _rxdata = _control = 0;
            _status = TDRE | CTS | DCD; // transmit data should be empty
        }

    private:
        void maybeSendNow(uint8_t data);
        uint8_t _control;
        uint8_t _status;
        uint8_t _txdata;
        uint8_t _rxdata;
        IRQ     _irq;
        SendToHost _send;
};

#endif /* M68B50_H_ */
