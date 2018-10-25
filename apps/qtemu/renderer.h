/*
 * Renderer.h
 *
 *  Created on: Oct 13, 2018
 *      Author: jmiller
 */

#ifndef RENDERER_H
#define RENDERER_H

#include <cstdlib>
#include <iostream>
#include <algorithm>
#include <vector>
#include <GL/glu.h>

// Abstract base class for renderer
class Renderer
{
public:
    typedef uint8_t Pixel;
    Renderer(int textureWidth, int textureHeight):
            _windowWidth(0), _windowHeight(0),
            _textureWidth(textureWidth), _textureHeight(textureHeight),
            _texture(textureWidth * textureHeight, 0), _lut(256, 0xffffffff) {
        for (int j = 0; j < textureHeight; j++) {
            for (int i = 0; i < textureWidth; i++) {
                _texture[j*textureWidth + i] = ((i^j) & 1) ? 255:0;
            }
        }
    }
    virtual ~Renderer() = default;
    virtual void initialize();  // Initialize GL state
    virtual void draw(); // Draw the current frame
    virtual const std::vector<Pixel>& getTexture() const { return _texture; }
    virtual const std::vector<uint32_t>& getLut() const { return _lut; }

    // Resize canvas to width x height
    virtual void resize(int width, int height);
    int getWindowWidth() const { return _windowWidth; }
    int getWindowHeight() const { return _windowHeight; }
    int getTextureWidth() const { return _textureWidth; }
    int getTextureHeight() const { return _textureHeight; }

    virtual void updateLut(const uint8_t* red, const uint8_t* green, const uint8_t* blue);
    virtual void updateScroll(int offsetX, int offsetY);
    virtual void updateZoom(int zoomX, int zoomY);
    void updateVideo(const uint8_t* video, int width, int height);

protected:
    int _windowWidth;
    int _windowHeight;
    int _textureWidth;
    int _textureHeight;
    int _scrollX = 0;
    int _scrollY = 0;
    int _zoomX = 1;
    int _zoomY = 1;
    std::vector<Pixel>  _texture;
    std::vector<uint32_t> _lut;
};

inline void checkGlError(const char* op) {
    for (GLenum error = glGetError(); error; error = glGetError()) {
        const GLubyte* msg = gluErrorString(error);
        std::cerr << "GlError(" << error << ") at '" << op << "' : \""  << msg << "\"" << std::endl;
    }
}

#endif // RENDERER_H
