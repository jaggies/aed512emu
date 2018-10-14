#-------------------------------------------------
#
# Project created by QtCreator 2017-02-25T14:32:11
#
#-------------------------------------------------

QT += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = qtemu
TEMPLATE = app
INCLUDEPATH += /opt/local/include
LIBS += -L/opt/local/lib -lglew -lGLU -lGL

SOURCES +=  \
    main.cpp\
    mainwindow.cpp \
    glwidget.cpp \
    renderer.cpp \
    drawpixelrenderer.cpp \
    gltexturerenderer.cpp \
    vbo.cpp \
    vertex.cpp

HEADERS  += \
    mainwindow.h \
    glwidget.h \
    renderer.h \
    drawpixelrenderer.h \
    gltexturerenderer.h \
    vbo.h \
    vertex.h

FORMS += mainwindow.ui
