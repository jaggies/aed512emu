/*
 * GLWidget.h
 *
 *  Created on: Oct 13, 2018
 *      Author: jmiller
 */

#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QOpenGLWidget>
#include <QWheelEvent>
#include <QTimer>
#include <iostream>
#include "renderer.h"

class GLWidget : public QOpenGLWidget
{
    Q_OBJECT
public:
    explicit GLWidget(QWidget *parent = Q_NULLPTR);
    virtual ~GLWidget() = default;

    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int w, int h) override;
    void wheelEvent(QWheelEvent *event) override;
    void mouseReleaseEvent ( QMouseEvent * event ) override;

    void play();
    bool isPlaying() const;

    Renderer* getRenderer() { return _renderer; }

public slots:
    void timeout();

private:
    Renderer* _renderer;
    QTimer* _timer;
    int _accumMouse;
};

#endif // GLWIDGET_H
