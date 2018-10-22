/*
 * MainWindow.cpp
 *
 *  Created on: Oct 13, 2018
 *      Author: jmiller
 */

#include <QtCore/QFileInfo>
#include "mainwindow.h"

#include "cputhread.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    glw = findChild<GLWidget*>(QString("openGLWidget"));
    statusBar()->setVisible(false);
    ioThread = new IoThread(this);
    cpuThread = new CpuThread(this);
    connect(cpuThread, &CpuThread::signal_redraw, this, &MainWindow::slot_redraw);
    connect(cpuThread, &CpuThread::finished, cpuThread, &QObject::deleteLater);
    connect(ioThread, &IoThread::signal_serial0_out, cpuThread, &CpuThread::slot_serial0_in);
    connect(ioThread, &IoThread::signal_serial1_out, cpuThread, &CpuThread::slot_serial1_in);
    connect(cpuThread, &CpuThread::signal_serial0_out, ioThread, &IoThread::slot_serial0_in);
    connect(cpuThread, &CpuThread::signal_serial1_out, ioThread, &IoThread::slot_serial1_in);
    connect(glw, &GLWidget::signal_key, cpuThread, &CpuThread::slot_key);
    cpuThread->start();
    ioThread->start();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void
MainWindow::on_actionClose_triggered(UNUSED bool checked) {
    std::cerr << "Close!\n";
}

