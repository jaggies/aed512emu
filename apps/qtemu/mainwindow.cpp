/*
 * MainWindow.cpp
 *
 *  Created on: Oct 13, 2018
 *      Author: jmiller
 */

#include <QFileInfo>
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
}

MainWindow::~MainWindow()
{
    delete ui;
}

void
MainWindow::on_actionOpen_triggered(UNUSED bool checked) {
    QString filename = QFileDialog::getOpenFileName(this, tr("Open study"), "", "Study files (*.std);;All files (*.*)");
}

void
MainWindow::on_actionClose_triggered(UNUSED bool checked) {
    if (glw) {
        glw->update();
    }
}

void
MainWindow::on_actionNew_triggered(UNUSED bool checked) {
}

void
MainWindow::updateUi() {
}
