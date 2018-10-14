/*
 * main.cpp
 *
 *  Created on: Oct 13, 2018
 *      Author: jmiller
 */

#include <QtWidgets/QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication application(argc, argv);
    MainWindow window;
    window.show();

    return application.exec();
}
