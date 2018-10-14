/*
 * Renederer.cpp
 *
 *  Created on: Oct 13, 2018
 *      Author: jmiller
 */

#include <cassert>
#include <GL/glew.h>
#include <GL/gl.h>
#include "renderer.h"

void Renderer::initialize() {
    glewInit();
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    const GLdouble du = 1.0;
    const GLdouble dv = 1.0;
    const GLdouble kNear = 0.0;
    const GLdouble kFar = 100.0;
    glOrtho(-du, du, -dv, dv, kNear, kFar);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void Renderer::draw() {
    glClearColor(0,0,1,1);
    glClear(GL_COLOR_BUFFER_BIT);
    glRasterPos2d(-1.0, -1.0);
}

void Renderer::resize(int width, int height) {
    _windowWidth = width;
    _windowHeight = height;
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    const GLdouble aspect = _windowWidth / _windowHeight;
    GLdouble du = std::max(1.0, aspect);
    GLdouble dv = std::max(1.0, 1.0 / aspect);
    GLdouble kNear = 0.0;
    GLdouble kFar = 100.0;
    glOrtho(0, du, 0, dv, kNear, kFar);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void Renderer::updateVideo(const uint8_t* video, int width, int height) {
    assert(width == _textureWidth);
    assert(height == _textureHeight);
    for (size_t i = 0; i < _texture.size(); i++) {
        _texture[i] = Pixel(*video, *video);
        video++;
    }
}

void Renderer::updateLut(const uint8_t* red, const uint8_t* blue, const uint8_t* green) {
    for (size_t i = 0; i < 256; i++) {
        _lut[i] = 0xff000000 | (blue[i] << 16) | (green[i] << 8) | red[i];
    }
}
