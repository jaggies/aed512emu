/*
 * WorkerThread.h
 *
 *  Created on: Oct 14, 2018
 *      Author: jmiller
 */

#ifndef APPS_QTEMU_WORKERTHREAD_H_
#define APPS_QTEMU_WORKERTHREAD_H_

#include <QtCore/QThread>
#include "cpu.h"
#include "clk.h"
#include "aedbus.h"

class WorkerThread: public QThread {
    Q_OBJECT
    public:
        explicit WorkerThread(QObject *parent = nullptr);
        virtual ~WorkerThread() override = default;
        void run() override;
        void stop() { _flag_stop = true; }

    signals:
        void handleVsync();
        void handleException(CPU::ExceptionType ex, int pc);

    private:
        typedef CLK Clock;
        CPU*    _cpu = nullptr;
        AedBus* _bus = nullptr;
        Clock*  _clk = nullptr;
        bool    _flag_stop = false;
};

#endif /* APPS_QTEMU_WORKERTHREAD_H_ */
