/*
 * DrawPixelRenderer.cpp
 *
 *  Created on: Oct 13, 2018
 *      Author: jmiller
 */

#include <GL/glew.h>
#include <GL/gl.h>
#include <iostream>
#include "drawpixelrenderer.h"

void DrawPixelRenderer::draw() {
    Renderer::draw();
    glPixelZoom(2.0f * _windowWidth / _textureWidth, 2.0f * getWindowHeight() / getTextureHeight());
    glDrawPixels(_textureWidth, _textureHeight, GL_LUMINANCE, GL_UNSIGNED_BYTE, &_texture[0]);
}

