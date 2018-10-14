/*
 * GlTextureRenderer.cpp
 *
 *  Created on: Oct 13, 2018
 *      Author: jmiller
 */

#include <iostream>
#include <cstddef> // offsetof()
#include <GL/glew.h>
#include <GL/gl.h>
#include "gltexturerenderer.h"
#include "vbo.h"

static const char _vertexShader[] =
    "#version 120\n"
    "struct State { mat4 modelview; mat4 projection; mat3 normal;};\n"
    "attribute vec4 position;"
    "attribute vec2 uv;\n"
    "varying vec2 v_uv;\n"
    "void main() {\n"
        "gl_Position = gl_ProjectionMatrix * gl_ModelViewMatrix * position;\n"
        "v_uv = uv;"
    "}\n";

static const char _fragmentShader[] =
    "varying vec2 v_uv;\n"
    "uniform sampler1D lut;\n"
    "uniform sampler2D image;\n"
    "void main() {\n"
        "vec4 raw = texture2D(image, v_uv);"
        "gl_FragColor = texture1D(lut, raw[0]);"
    "}\n";

void GlTextureRenderer::initialize() {
    Renderer::initialize();
    _program = createProgram(_vertexShader, _fragmentShader);
    if (!_program) {
        std::cout << "Could not create program." << std::endl;
        return;
    }

    glUseProgram(_program);
    _positionHandle = glGetAttribLocation(_program, "position");
    checkGlError("glGetAttribLocation(position)");
    _colorHandle = glGetAttribLocation(_program, "color");
    checkGlError("glGetAttribLocation(color)");
    _imageUniform = glGetUniformLocation(_program, "image");
    checkGlError("glGetUniformLocation(image)");
    _lutUniform = glGetUniformLocation(_program, "lut");
    checkGlError("glGetUniformLocation(lut)");

    std::vector<Vertex> data;
    data.push_back(Vertex(-2.0f, -2.0f, 0.0f, -2.0f, -2.0f)); // position 3, color 3
    data.push_back(Vertex( 2.0f, -2.0f, 0.0f, 2.0f, -2.0f));
    data.push_back(Vertex( 2.0f,  2.0f, 0.0f, 2.0f, 2.0f));
    data.push_back(Vertex(-2.0f,  2.0f, 0.0f, -2.0f, 2.0f));

    std::vector<Vbo<Vertex>::Attr> attrs;
    attrs.push_back(Vbo<Vertex>::Attr("position", 3, offsetof(Vertex, position)));
    attrs.push_back(Vbo<Vertex>::Attr("uv", 2, offsetof(Vertex, uv)));
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
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG, _textureWidth, _textureHeight, 0, GL_RG, GL_UNSIGNED_BYTE, nullptr);
    checkGlError("after glTexImage2D");

    glActiveTexture(GL_TEXTURE1);
    glUniform1i(_lutUniform, 1 /* texture unit 1 */);
    glGenTextures(1, &_lutTexture);
    glBindTexture(GL_TEXTURE_1D, _lutTexture);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    // Only the first 64 entries (6-bits) are meaningful in this texture
    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    checkGlError("after glTexImage1D (luma)");

    glBindTexture(GL_TEXTURE_1D, _lutTexture);

    // Default to gray ramp
    uint8_t lumaLut[256][4];
    for (int i = 0; i < 256; i++) {
        lumaLut[i][0] = lumaLut[i][1] = lumaLut[i][2] =  uint8_t(i);
        lumaLut[i][3] = 255;
    }
    glTexSubImage1D(GL_TEXTURE_1D, 0, 0, sizeof(lumaLut)/sizeof(lumaLut[0]), GL_RGBA, GL_UNSIGNED_BYTE, &lumaLut[0][0]);

    glEnable(GL_TEXTURE_1D);
    glEnable(GL_TEXTURE_2D);
}

void GlTextureRenderer::draw() {
    Renderer::draw();
    glUseProgram(_program);
    checkGlError("glUseProgram");
    glBindTexture(GL_TEXTURE_2D, _imageTexture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, _textureWidth, _textureHeight, GL_RG, GL_UNSIGNED_BYTE, &getTexture()[0]);
    checkGlError("after glTexSubImage");
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
        checkGlError("glAttachShader");
        glAttachShader(program, pixelShader);
        checkGlError("glAttachShader");
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
