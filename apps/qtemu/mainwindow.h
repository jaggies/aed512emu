/*
 * MainWindow.h
 *
 *  Created on: Oct 13, 2018
 *      Author: jmiller
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <iostream>
#include <QMainWindow>
#include <QString>
#include <QFileDialog>
#include <QSpinBox>
#include <QStatusBar>
#include "glwidget.h"

#define UNUSED __attribute__((unused))

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    virtual ~MainWindow();

public slots:
    void on_actionOpen_triggered(UNUSED bool checked);
    void on_actionClose_triggered(UNUSED bool checked);
    void on_actionNew_triggered(UNUSED bool checked);

private:
    void updateUi();

    Ui::MainWindow *ui;
    GLWidget* glw;
};

#endif // MAINWINDOW_H
