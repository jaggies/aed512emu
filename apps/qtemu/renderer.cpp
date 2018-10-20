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
    glOrtho(0, du, 0, dv, kNear, kFar);
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
    std::cerr << "Window: " << std::dec << width << "x" << height << std::endl;
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    const GLdouble aspect = 1.0; //(double) _windowWidth / _windowHeight;
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
        _texture[i] = *video;
        video++;
    }
}

void Renderer::updateLut(const uint8_t* red, const uint8_t* green, const uint8_t* blue) {
    for (size_t i = 0; i < 256; i++) {
        _lut[i] = 0xff000000 | (blue[i] << 16) | (green[i] << 8) | red[i];
    }
}

void Renderer::updateScroll(int offsetX, int offsetY) {
    _scrollX = offsetX;
    _scrollY = _textureHeight/2 + offsetY; // TODO: WHY!??!?!?!
}

void Renderer::updateZoom(int zoomX, int zoomY) {
    _zoomX = zoomX;
    _zoomY = zoomY;
}
