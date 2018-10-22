/*
 * GlTextureRenderer.cpp
 *
 *  Created on: Oct 13, 2018
 *      Author: jmiller
 */

#include <iostream>
#include <cstddef> // offsetof()
#include <cassert>
#include <GL/glew.h>
#include <GL/gl.h>
#include "gltexturerenderer.h"
#include "vbo.h"
#include "coreutil.h"

static bool debug = true;

static const char _vertexShader[] =
    "#version 120\n"
    "attribute vec4 position;"
    "attribute vec4 uv;\n"
    "void main() {\n"
        "gl_Position = gl_ProjectionMatrix * gl_ModelViewMatrix * position;\n"
        "gl_TexCoord[0] = gl_TextureMatrix[0] * uv;" //gl_MultiTexCoord0
    "}\n";

static const char _fragmentShader[] =
    "uniform sampler1D lut;\n"
    "uniform sampler2D image;\n"
    "void main() {\n"
        "vec4 raw = texture2D(image, vec2(gl_TexCoord[0][0], gl_TexCoord[0][1]));"
        "gl_FragColor = texture1D(lut, raw[0]);"
    "}\n";

void GlTextureRenderer::initialize() {
    Renderer::initialize();
    _program = createProgram(_vertexShader, _fragmentShader);
    if (!_program) {
        std::cerr << "Could not create program." << std::endl;
        return;
    } else if (debug) {
        std::cerr << "Successfully created program" << std::endl;
    }

    glUseProgram(_program);
    checkGlError("glUseProgram in initialize()");
    _positionHandle = glGetAttribLocation(_program, "position");
    checkGlError("glGetAttribLocation(position)");
    _imageUniform = glGetUniformLocation(_program, "image");
    checkGlError("glGetUniformLocation(image)");
    _lutUniform = glGetUniformLocation(_program, "lut");
    checkGlError("glGetUniformLocation(lut)");

    std::vector<Vertex> data;
    data.push_back(Vertex(0.0f, 0.0f, 0.0f, 0.0f, 0.0f)); // position 3, color 3
    data.push_back(Vertex(1.0f, 0.0f, 0.0f, 1.0f, 0.0f));
    data.push_back(Vertex(1.0f, 1.0f, 0.0f, 1.0f, 1.0f));
    data.push_back(Vertex(0.0f, 1.0f, 0.0f, 0.0f, 1.0f));

    std::vector<Vbo<Vertex>::Attr> attrs;
    attrs.push_back(Vbo<Vertex>::Attr("position", Number(Vertex::position), offsetof(Vertex, position)));
    attrs.push_back(Vbo<Vertex>::Attr("uv", Number(Vertex::uv), offsetof(Vertex, uv)));
    _vbo = new Vbo<Vertex>(_program, GL_QUADS, data, attrs);

    glShadeModel(GL_FLAT);
    glDisable(GL_DEPTH_TEST);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glActiveTexture(GL_TEXTURE0);
    glUniform1i(_imageUniform, 0 /* texture unit 0 */);
    glGenTextures(1, &_imageTexture);
    glBindTexture(GL_TEXTURE_2D, _imageTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, _textureWidth, _textureHeight, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);
    checkGlError("after glTexImage2D");

    glActiveTexture(GL_TEXTURE1);
    glUniform1i(_lutUniform, 1 /* texture unit 1 */);
    glGenTextures(1, &_lutTexture);
    glBindTexture(GL_TEXTURE_1D, _lutTexture);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    checkGlError("after glTexImage1D (lut)");

    glEnable(GL_TEXTURE_1D);
    glEnable(GL_TEXTURE_2D);
}

void GlTextureRenderer::draw() {
    Renderer::draw();
    glUseProgram(_program);
    checkGlError("glUseProgram");

    glBindTexture(GL_TEXTURE_2D, _imageTexture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, _textureWidth, _textureHeight, GL_RED, GL_UNSIGNED_BYTE,
            &getTexture()[0]);

    glBindTexture(GL_TEXTURE_1D, _lutTexture);
    glTexSubImage1D(GL_TEXTURE_1D, 0, 0, _lut.size(), GL_RGBA, GL_UNSIGNED_BYTE,
            &getLut()[0]);
    checkGlError("after glTexSubImage");

    glActiveTexture(GL_TEXTURE0);
    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();
    glScalef(1.0f / _zoomX, 1.0 / _zoomY, 1.0);
    glTranslatef((float) _scrollX / _textureWidth, (float) _scrollY / _textureHeight, 0);
    checkGlError("after texture transform");

    _vbo->draw();
    checkGlError("after vbo.draw()");
}

GLuint GlTextureRenderer::loadShader(GLenum shaderType, const char* source) {
    GLuint shader = glCreateShader(shaderType);
    if (shader) {
        glShaderSource(shader, 1, &source, nullptr);
        glCompileShader(shader);
        GLint compiled = 0;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
        if (!compiled) {
            GLint infoLen = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
            if (infoLen) {
                char* buf = new char[infoLen];
                if (buf) {
                    glGetShaderInfoLog(shader, infoLen, nullptr, buf);
                    std::cerr << "Could not compile shader " << shaderType << ", msg=" << buf << std::endl;
                    delete []buf;
                }
                glDeleteShader(shader);
                shader = 0;
            }
        }
    }
    return shader;
}

GLuint GlTextureRenderer::createProgram(const char* pVertexSource, const char* pFragmentSource) {
    GLuint vertexShader = loadShader(GL_VERTEX_SHADER, pVertexSource);
    if (!vertexShader) {
        std::cerr << "Failed to load VERTEX shader" << std::endl;
        return 0;
    }

    GLuint pixelShader = loadShader(GL_FRAGMENT_SHADER, pFragmentSource);
    if (!pixelShader) {
        std::cerr << "Failed to load FRAGMENT shader" << std::endl;
        return 0;
    }

    GLuint program = glCreateProgram();
    if (program) {
        glAttachShader(program, vertexShader);
        checkGlError("vertexShader");
        glAttachShader(program, pixelShader);
        checkGlError("fragmentShader");
        glLinkProgram(program);
        GLint linkStatus = GL_FALSE;
        glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
        if (linkStatus != GL_TRUE) {
            GLint bufLength = 0;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &bufLength);
            if (bufLength) {
                char* buf = new char[bufLength];
                if (buf) {
                    glGetProgramInfoLog(program, bufLength, nullptr, buf);
                    std::cerr << "Could not link program:\n" << buf << std::endl;
                    delete [] buf;
                }
            }
            glDeleteProgram(program);
            program = 0;
        }
    }
    return program;
}
