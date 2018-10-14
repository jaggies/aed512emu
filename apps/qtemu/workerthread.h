/*
 * WorkerThread.h
 *
 *  Created on: Oct 14, 2018
 *      Author: jmiller
 */

#ifndef APPS_QTEMU_WORKERTHREAD_H_
#define APPS_QTEMU_WORKERTHREAD_H_

#include <QtCore/QThread>

class WorkerThread: public QThread {
    Q_OBJECT
    public:
        explicit WorkerThread(QObject *parent = nullptr) :
                QThread(parent) {
        }
        void run() override {
            while (1) {
                sleep(1);
                emit vsync();
            }
        }
    signals:
        void vsync();
};

#endif /* APPS_QTEMU_WORKERTHREAD_H_ */
