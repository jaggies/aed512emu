/*
 * DrawPixelRenderer.h
 *
 *  Created on: Oct 13, 2018
 *      Author: jmiller
 */

#ifndef DRAWPIXELRENDERER_H
#define DRAWPIXELRENDERER_H

#include "renderer.h"

class DrawPixelRenderer : public Renderer
{
public:
    DrawPixelRenderer() { }
    virtual ~DrawPixelRenderer() override = default;
    virtual void draw() override; 
};

#endif // DRAWPIXELRENDERER_H
