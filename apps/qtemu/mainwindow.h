/*
 * MainWindow.h
 *
 *  Created on: Oct 13, 2018
 *      Author: jmiller
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <iostream>
#include <QtWidgets/QMainWindow>
#include <QtCore/QString>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QStatusBar>
#include "glwidget.h"
#include "workerthread.h"

#define UNUSED __attribute__((unused))

namespace Ui {
class MainWindow;
}

class MainWindow: public QMainWindow {
    Q_OBJECT
    public:
        explicit MainWindow(QWidget *parent = nullptr);
        virtual ~MainWindow();
    public slots:
        void on_actionClose_triggered(UNUSED bool checked);
        void handleRedraw(const uint8_t* video, const uint8_t* red, const uint8_t * green,
                const uint8_t *blue, int width, int height) {
            //std::cerr << "HandleRedraw: " << std::dec << width << "x" << height << std::endl;
            glw->updateVideo(video, width, height);
            glw->updateLut(red, green, blue);
            glw->update();
        }
        void closeEvent(QCloseEvent *event) {
            worker->stop();
            worker->wait();
        }
    private:
        Ui::MainWindow* ui;
        GLWidget*       glw;
        WorkerThread*   worker;
};

#endif // MAINWINDOW_H
