/*
 * IoThread.cpp
 *
 *  Created on: Oct 22, 2018
 *      Author: jmiller
 */

#include <unistd.h>
#include <fcntl.h>
#include "iothread.h"

IoThread::IoThread(QObject *parent): QThread(parent) {

}

void IoThread::run() {
    int fd = open("input", O_RDONLY); // | O_NONBLOCK);
    if (fd > 0) {
        while (!_flag_stop) {
            char ch;
            long n = ::read(fd, &ch, 1);
            if (n > 0) {
                emit signal_serial0_out(ch);
            }
        }
    } else {
        std::cerr << "Can't open 'input'" << std::endl;
    }
}
