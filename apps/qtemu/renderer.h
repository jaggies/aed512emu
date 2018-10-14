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
#include <GL/gl.h>

// Abstract base class for renderer
class Renderer
{
public:
    Renderer(int width = 512, int height = 512):
            _windowWidth(0), _windowHeight(0),
            _textureWidth(width), _textureHeight(height),
            _texture(width * height, 0) { }
    virtual ~Renderer() = default;
    virtual void initialize();  // Initialize GL state
    virtual void draw(); // Draw the current frame
    virtual const std::vector<uint8_t>& getTexture() const { return _texture; }

    // Resize canvas to width x height
    virtual void resize(int width, int height);
    int getWindowWidth() const { return _windowWidth; }
    int getWindowHeight() const { return _windowHeight; }
    int getTextureWidth() const { return _textureWidth; }
    int getTextureHeight() const { return _textureHeight; }

protected:
    int _windowWidth;
    int _windowHeight;
    int _textureWidth;
    int _textureHeight;
    std::vector<uint8_t> _texture;
};

inline void checkGlError(const char* op) {
    for (GLenum error = glGetError(); error; error = glGetError()) {
        std::cerr << "after " << op << " glError=" << error << std::endl;
    }
}

#endif // RENDERER_H
