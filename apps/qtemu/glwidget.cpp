/*
 * GLWidget.h
 *
 *  Created on: Oct 13, 2018
 *      Author: jmiller
 */


#include <QtCore/QEvent>
#include <QtGui/QMouseEvent>
#include <QtGui/QWheelEvent>
#include <QtGui/QSurfaceFormat>
#include <GL/glew.h>
#include <GL/gl.h>
#include "glwidget.h"
#include "drawpixelrenderer.h"
#include "gltexturerenderer.h"

static bool debug = false;

GLWidget::GLWidget(QWidget *parent) : QOpenGLWidget(parent), _accumMouse(0)
{
    QSurfaceFormat sformat;
    sformat.setProfile(QSurfaceFormat::CoreProfile);
    sformat.setMajorVersion(2);
    sformat.setMinorVersion(1);
    sformat.setDepthBufferSize(0);
    sformat.setAlphaBufferSize(0);
    sformat.setRedBufferSize(8);
    sformat.setGreenBufferSize(8);
    sformat.setBlueBufferSize(8);
    sformat.setStencilBufferSize(0);
    sformat.setSwapBehavior(QSurfaceFormat::SingleBuffer);
    sformat.setStereo(false);
    setFormat(sformat);

    _renderer = new GlTextureRenderer(512, 512); // TODO
    _timer = new QTimer();
    connect(_timer, SIGNAL(timeout()), this, SLOT(timeout()));

    setMouseTracking(true);
}

void GLWidget::initializeGL() {
    if (debug) std::cerr << __func__ << std::endl;
    _renderer->initialize();
}

void GLWidget::paintGL() {
    if (debug) std::cerr << __func__ << std::endl;
    _renderer->draw();
}

void GLWidget::resizeGL(int width, int height) {
    if (debug) std::cerr << __func__ << std::endl;
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

void GLWidget::mouseMoveEvent(QMouseEvent *event) {
    const int x = event->pos().x() * 4096 / (width() - 1);
    const int y = event->pos().y() * 4096 / (height() - 1);
    emit signal_mouseMove(x, y);
}

void GLWidget::keyPressEvent(QKeyEvent* event) {
    emit signal_key(event);
}

void GLWidget::keyReleaseEvent(QKeyEvent* event) {
    emit signal_key(event);
}


