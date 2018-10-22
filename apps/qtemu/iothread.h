/*
 * IoThread.h
 *
 *  Created on: Oct 22, 2018
 *      Author: jmiller
 */

#ifndef APPS_QTEMU_IOTHREAD_H_
#define APPS_QTEMU_IOTHREAD_H_
#include <iostream>
#include <QtCore/QThread>

class IoThread : public QThread {
    Q_OBJECT
    public:
        explicit IoThread(QObject *parent = nullptr);
        virtual ~IoThread() override = default;
        void run() override;
        void stop() { _flag_stop = true; }

    public slots:
        void slot_serial0_in(char ch) {
            std::cout << ch << std::endl;
        }
        void slot_serial1_in(char ch) {
            std::cout << ch << std::endl;
        }

    signals:
        void signal_serial0_out(char ch);
        void signal_serial1_out(char ch);

    private:
        bool _flag_stop = false;
};

#endif /* APPS_QTEMU_IOTHREAD_H_ */
