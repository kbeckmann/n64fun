// Copyright (c) 2021 Konrad Beckmann
// SPDX-License-Identifier: MIT

#include "vertex.h"
#include "vector.h"

void vertex_init(Vertex *out, Vector4f pos)
{
    out->pos = pos;
}

void vertex_transform(const Matrix4f *m, const Vertex *in, Vertex *out)
{
    vector_transform(m, &in->pos, &out->pos);
}

void vertex_perspective_divide(Vertex *io)
{
    float w = io->pos.w;

    io->pos.x /= w;
    io->pos.y /= w;
    io->pos.z /= w;
    // Keep w unchanged
}

float vertex_triangle_area2(const Vertex *a, const Vertex *b, const Vertex *c)
{
    float x1 = b->pos.x - a->pos.x;
    float y1 = b->pos.y - a->pos.y;

    float x2 = c->pos.x - a->pos.x;
    float y2 = c->pos.y - a->pos.y;

    return (x1 * y2 - x2 * y1);
}
