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
#include "cputhread.h"
#include "iothread.h"
#include "glwidget.h"

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
        void slot_redraw(const uint8_t* video, const uint8_t* red, const uint8_t * green,
                const uint8_t *blue, const int* zoom, const int* scroll, int width, int height) {
            Renderer* renderer = glw->getRenderer();
            renderer->updateVideo(video, width, height);
            renderer->updateLut(red, green, blue);
            renderer->updateScroll(scroll[0], scroll[1]);
            renderer->updateZoom(zoom[0], zoom[1]);
            glw->update();
        }
        void closeEvent(QCloseEvent *event) {
            cpuThread->stop();
            cpuThread->wait();
            ioThread->stop();
            ioThread->wait();
        }
    private:
        Ui::MainWindow* ui = nullptr;
        GLWidget*       glw = nullptr;
        CpuThread*      cpuThread = nullptr;
        IoThread*       ioThread = nullptr;
};

#endif // MAINWINDOW_H
