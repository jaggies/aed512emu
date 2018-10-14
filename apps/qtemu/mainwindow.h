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
    void on_actionClose_triggered(UNUSED bool checked);

    void idle() {
        std::cerr << "idle!\n";
    }
private:
    Ui::MainWindow *ui;
    GLWidget* glw;
};

#endif // MAINWINDOW_H
