/*
 * vbo.h
 *
 *  Created on: Oct 13, 2018
 *      Author: jmiller
 */

#ifndef VBO_H
#define VBO_H

#include <vector>

template <typename T>
class Vbo {
    public:
        class Attr {
            public:
                Attr() { }
                Attr(const char* name, GLint size, size_t offset)
                    : _name(name), _size(size), _offset(offset), _handle(-1) { }
                std::string _name;
                GLint _size = 0;
                size_t _offset = 0;
                GLint _handle = 0;
        };
        Vbo(GLuint program, GLint mode, const std::vector<T>& data, const std::vector<Attr>& attrs)
            : _program(program), _mode(mode), _data(data), _attrs(attrs) {
            glGenBuffers(1, &_vboId);
            glBindBuffer(GL_ARRAY_BUFFER, _vboId);
            glBufferData(GL_ARRAY_BUFFER, _data.size() * sizeof(T), &_data[0], GL_STATIC_DRAW);
            resolve(_program);
        }
        virtual ~Vbo() { glDeleteBuffers(1, &_vboId); _vboId = 0; }
        void draw() {
            glBindBuffer(GL_ARRAY_BUFFER, _vboId);
            for (size_t i = 0; i < _attrs.size(); i++) {
                Attr& attr = _attrs[i];
                glVertexAttribPointer(attr._handle, attr._size, GL_FLOAT, false, sizeof(T),
                            (void*) attr._offset);
                checkGlError("glVertexAttribPointer");
                glEnableVertexAttribArray(attr._handle);
                checkGlError("glEnableVertexAttribArray");
            }
            glDrawArrays(_mode, 0, _data.size());
        }
    private:
        void resolve(GLint program) {
            // Resolve symbols
            for (size_t i = 0; i < _attrs.size(); i++) {
                Attr& attr = _attrs[i];
                attr._handle = glGetAttribLocation(program, attr._name.c_str());
                if (attr._handle < 0) {
                    std::cout << "Couldn't find handle for '" << attr._name << "'\n";
                }
            }
        }
        GLint _program;
        GLint _mode;
        GLuint _vboId;
        std::vector<T> _data;
        std::vector<Attr> _attrs;
};

#endif // VBO_H
