/*
 * GLWidget.h
 *
 *  Created on: Oct 13, 2018
 *      Author: jmiller
 */

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

void GLWidget::mouseReleaseEvent ( QMouseEvent * event ) {
    std::cerr << "Mouse release: " << event->button() << std::endl;
}

bool GLWidget::isPlaying() const {
    return _timer->isActive();
}

void GLWidget::timeout() {

}
