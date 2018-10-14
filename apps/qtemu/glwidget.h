/*
 * GLWidget.h
 *
 *  Created on: Oct 13, 2018
 *      Author: jmiller
 */

#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QtWidgets/QOpenGLWidget>
#include <QtGui/QWheelEvent>
#include <QtCore/QTimer>
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
        void mousePressEvent ( QMouseEvent * event ) override;
        void mouseReleaseEvent ( QMouseEvent * event ) override;
        void keyPressEvent(QKeyEvent* event) override;
        void keyReleaseEvent(QKeyEvent* event) override;

        void updateVideo(const uint8_t* video, int width, int height);
        void updateLut(const uint8_t* red, const uint8_t* blue, const uint8_t* green);

        void play();
        bool isPlaying() const;

        Renderer* getRenderer() { return _renderer; }

    public slots:
        void timeout() { }

    signals:
        void key(QKeyEvent* event);

    private:
        Renderer* _renderer;
        QTimer* _timer;
        int _accumMouse;
};

#endif // GLWIDGET_H
