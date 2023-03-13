// Copyright (c) 2021 Konrad Beckmann
// SPDX-License-Identifier: MIT

#include "vector.h"

void vector_transform(const Matrix4f *m, const Vector4f *in, Vector4f *out)
{
    Vector4f tmp;

    tmp.x = m->m[0][0] * in->x + m->m[1][0] * in->y + m->m[2][0] * in->z + m->m[3][0] * in->w;
    tmp.y = m->m[0][1] * in->x + m->m[1][1] * in->y + m->m[2][1] * in->z + m->m[3][1] * in->w;
    tmp.z = m->m[0][2] * in->x + m->m[1][2] * in->y + m->m[2][2] * in->z + m->m[3][2] * in->w;
    tmp.w = m->m[0][3] * in->x + m->m[1][3] * in->y + m->m[2][3] * in->z + m->m[3][3] * in->w;

    *out = tmp;
}

float vector_dot(const Vector4f *a, const Vector4f *b)
{
    return (
        a->x * b->x +
        a->y * b->y +
        a->z * b->z +
        a->w * b->w
    );
}

void vector_add(const Vector4f *a, const Vector4f *b, Vector4f *out)
{
    out->x = a->x + b->x;
    out->y = a->y + b->y;
    out->z = a->z + b->z;
    out->w = a->w + b->w;
}
