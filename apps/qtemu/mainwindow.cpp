/*
 * MainWindow.cpp
 *
 *  Created on: Oct 13, 2018
 *      Author: jmiller
 */

#include <QtCore/QFileInfo>
#include "mainwindow.h"
#include "ui_mainwindow.h"

#define Number(a) (sizeof(a) / sizeof(a[0]))

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    glw = findChild<GLWidget*>(QString("openGLWidget"));
    statusBar()->setVisible(false);
    QTimer::singleShot(0, this, SLOT(idle()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void
MainWindow::on_actionClose_triggered(UNUSED bool checked) {
    std::cerr << "Close!\n";
}

