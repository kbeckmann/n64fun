// Copyright (c) 2021 Konrad Beckmann
// Copyright (c) 2021 SiliconSloth
// SPDX-License-Identifier: MIT
// Based on https://github.com/SiliconSloth/tri3d/blob/master/src/matrix.c

#include <math.h>

#include "matrix.h"

// Column-major

void matrix_mul(const Matrix4f *a, const Matrix4f *b, Matrix4f *out)
{
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            float sum = 0.0f;
            for (int k = 0; k < 4; k++) {
                sum += a->m[k][i] * b->m[j][k];
            }
            out->m[j][i] = sum;
        }
    }
}

void matrix_scale(Matrix4f *out, float x, float y, float z)
{
    LOAD_MATRIX(out->m,
        x, 0, 0, 0,
        0, y, 0, 0,
        0, 0, z, 0,
        0, 0, 0, 1
    );
}

void matrix_translate(Matrix4f *out, float x, float y, float z)
{
    LOAD_MATRIX(out->m,
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        x, y, z, 1
    );
}

void matrix_rotate_x(Matrix4f *out, float angle)
{
    float sin = sinf(angle);
    float cos = cosf(angle);

    LOAD_MATRIX(out->m,
        1,    0,   0, 0,
        0,  cos, sin, 0,
        0, -sin, cos, 0,
        0,    0,   0, 1
    );
}

void matrix_rotate_y(Matrix4f *out, float angle)
{
    float sin = sinf(angle);
    float cos = cosf(angle);

    LOAD_MATRIX(out->m,
         cos, 0, sin, 0,
           0, 1,   0, 0,
        -sin, 0, cos, 0,
           0, 0,   0, 1
    );
}

void matrix_rotate_z(Matrix4f *out, float angle)
{
    float sin = sinf(angle);
    float cos = cosf(angle);

    LOAD_MATRIX(out->m,
         cos, sin, 0, 0,
        -sin, cos, 0, 0,
           0,   0, 1, 0,
           0,   0, 0, 1
    );
}

void matrix_perspective(Matrix4f *out, float fov, float aspect, float near, float far)
{
    float tanHalfFOV = tanf(fov / 2);
    float hs = 1 / (tanHalfFOV * aspect);
    float vs = 1 / (tanHalfFOV);

    LOAD_MATRIX(out->m,
        hs,  0,                          0, 0,
         0, vs,                          0, 0,
         0,  0,         far / (far - near), 1,
         0,  0, -far * near / (far - near), 0
    );
}

void matrix_screen_space_transform(Matrix4f *out, float halfWidth, float halfHeight)
{
    LOAD_MATRIX(out->m,
        halfWidth,           0, 0, 0,
                0, -halfHeight, 0, 0,
                0,           0, 1, 0,
        halfWidth,  halfHeight, 0, 1
    );
}

void matrix_identity(Matrix4f *out)
{
    LOAD_MATRIX(out->m,
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    );
}
