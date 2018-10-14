/*
 * MainWindow.cpp
 *
 *  Created on: Oct 13, 2018
 *      Author: jmiller
 */

#include <QtCore/QFileInfo>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "workerthread.h"

#define Number(a) (sizeof(a) / sizeof(a[0]))

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    glw = findChild<GLWidget*>(QString("openGLWidget"));
    statusBar()->setVisible(false);

    WorkerThread *workerThread = new WorkerThread(this);
    connect(workerThread, &WorkerThread::vsync, this, &MainWindow::handleVsync);
    connect(workerThread, &WorkerThread::finished, workerThread, &QObject::deleteLater);
    workerThread->start();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void
MainWindow::on_actionClose_triggered(UNUSED bool checked) {
    std::cerr << "Close!\n";
}

