#ifndef GLTEXTURERENDERER_H
#define GLTEXTURERENDERER_H

#include "renderer.h"
#include "vbo.h"
#include "vertex.h"

class GlTextureRenderer : public Renderer
{
public:
    GlTextureRenderer(int textureWidth, int textureHeight) : Renderer(textureWidth, textureHeight) { }
    virtual ~GlTextureRenderer() override = default;
    void draw() override;
    void initialize() override;
private:
    GLuint  loadShader(GLenum shaderType, const char* shaderSource);
    GLuint  createProgram(const char* vertexSource, const char* fragmentSource);
    GLuint  _program = 0;
    GLuint  _lutTexture = 0;
    GLint   _lutUniform = 0;
    GLint   _imageUniform = 0;
    GLuint  _imageTexture = 0;
    GLint   _positionHandle = 0;

    Vbo<Vertex>* _vbo = nullptr;
};

#endif // GLTEXTURERENDERER_H
