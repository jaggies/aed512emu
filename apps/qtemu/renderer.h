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

// Abstract base class for renderer
class Renderer
{
public:
    struct Pixel {
        Pixel(uint8_t r, uint8_t g) : red(r), green(g) { }
        uint8_t red;
        uint8_t green;
    };
    Renderer(int width = 512, int height = 512):
            _windowWidth(0), _windowHeight(0),
            _textureWidth(width), _textureHeight(height),
            _texture(width * height, Pixel(0,0)), _lut(256, 0xffffffff) {
        for (int j = 0; j < height; j++) {
            for (int i = 0; i < width; i++) {
                _texture[j*width + i] = Pixel(i, j);
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

    virtual void updateScroll(int offsetX, int offsetY) { }
    virtual void updateZoom(int zoomX, int zoomY) { }
    void updateVideo(const uint8_t* video, int width, int height);
    void updateLut(const uint8_t* red, const uint8_t* green, const uint8_t* blue);


protected:
    int _windowWidth;
    int _windowHeight;
    int _textureWidth;
    int _textureHeight;
    std::vector<Pixel>  _texture;
    std::vector<uint32_t> _lut;
};

inline void checkGlError(const char* op) {
    for (GLenum error = glGetError(); error; error = glGetError()) {
        std::cerr << "after " << op << " glError=" << error << std::endl;
    }
}

#endif // RENDERER_H
