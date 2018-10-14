/*
 * GLWidget.h
 *
 *  Created on: Oct 13, 2018
 *      Author: jmiller
 */


#include <QtCore/QEvent>
#include <QtGui/QMouseEvent>
#include <QtGui/QWheelEvent>
#include <GL/glew.h>
#include <GL/gl.h>
#include "glwidget.h"
#include "drawpixelrenderer.h"
#include "gltexturerenderer.h"

GLWidget::GLWidget(QWidget *parent) : QOpenGLWidget(parent), _accumMouse(0)
{
    _renderer = new GlTextureRenderer();
    _timer = new QTimer();
    connect(_timer, SIGNAL(timeout()), this, SLOT(timeout()));
}

void GLWidget::initializeGL() {
    _renderer->initialize();
}

void GLWidget::paintGL() {
    _renderer->draw();
}

void GLWidget::resizeGL(int width, int height) {
    _renderer->resize(width, height);
}

void GLWidget::wheelEvent(QWheelEvent *event) {
    _accumMouse += event->orientation() == Qt::Horizontal ? -event->delta() : event->delta();
    update();
}

void GLWidget::mousePressEvent ( QMouseEvent * event ) {
    std::cerr << "Mouse press: " << event->button() << std::endl;
}

void GLWidget::mouseReleaseEvent ( QMouseEvent * event ) {
    std::cerr << "Mouse release: " << event->button() << std::endl;
}

void GLWidget::keyPressEvent(QKeyEvent* event) {
    emit key(event);
}

void GLWidget::keyReleaseEvent(QKeyEvent* event) {
    emit key(event);
}

void GLWidget::updateVideo(const uint8_t* video, int width, int height) {
    _renderer->updateVideo(video, width, height);
}

void GLWidget::updateLut(const uint8_t* red, const uint8_t* blue, const uint8_t* green) {
    _renderer->updateLut(red, green, blue);
}

