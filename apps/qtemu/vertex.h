/*
 * vertex.cpp
 *
 *  Created on: Oct 13, 2018
 *      Author: jmiller
 */

#ifndef VERTEX_H
#define VERTEX_H

struct Vertex {
    Vertex(float x, float y, float z, float u, float v, float s = 0.0, float t = 1.0) {
        position[0] = x; position[1] = y; position[2] = z; position[3] = 1;
        uv[0] = u; uv[1] = v; uv[2] = s; uv[3] = t;
    }
    float position[4];
    float uv[4];
};

#endif // VERTEX_H
