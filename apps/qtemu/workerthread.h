/*
 * WorkerThread.h
 *
 *  Created on: Oct 14, 2018
 *      Author: jmiller
 */

#ifndef APPS_QTEMU_WORKERTHREAD_H_
#define APPS_QTEMU_WORKERTHREAD_H_

#include <QtCore/QThread>
#include <QtGui/QMouseEvent>
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
        void handleRedraw(const uint8_t* video, const uint8_t* red, const uint8_t * green,
                const uint8_t *blue, const int* zoom, const int* scroll, int width, int height);
        void handleException(CPU::ExceptionType ex, int pc);

    public slots:
        void key(QKeyEvent* event) {
            //    int qtmods = event->modifiers();
            //    const bool meta = qtmods & Qt::MetaModifier;
            //    const bool shift = qtmods & Qt::ShiftModifier;
            //    const bool ctrl = qtmods & Qt::ControlModifier;
            //    const bool alt = qtmods & Qt::AltModifier;
            const QString& str = event->text();
            if (event->type() == QKeyEvent::KeyPress && str.length() > 0) {
                int key = (int) str.at(0).toLatin1();
                _bus->keyDown(key);
            }
        }

    private:
        typedef CLK Clock;
        CPU*    _cpu = nullptr;
        AedBus* _bus = nullptr;
        Clock*  _clk = nullptr;
        bool    _flag_stop = false;
};

#endif /* APPS_QTEMU_WORKERTHREAD_H_ */
