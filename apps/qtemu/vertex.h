/*
 * vertex.cpp
 *
 *  Created on: Oct 13, 2018
 *      Author: jmiller
 */

#ifndef VERTEX_H
#define VERTEX_H

struct Vertex {
    Vertex(float x, float y, float z, float u, float v) {
        position[0] = x; position[1] = y; position[2] = z;
        uv[0] = u; uv[1] = v;
    }
    float position[3];
    float uv[2];
};

#endif // VERTEX_H
