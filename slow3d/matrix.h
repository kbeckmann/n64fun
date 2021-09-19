// Copyright (c) 2021 Konrad Beckmann
// Copyright (c) 2021 SiliconSloth
// Based on https://github.com/SiliconSloth/tri3d/blob/master/include/matrix.h
// SPDX-License-Identifier: MIT

#pragma once

#include "types.h"

void matrix_mul(const Matrix4f *a, const Matrix4f *b, Matrix4f *out);

void matrix_scale(Matrix4f *out, float x, float y, float z);
void matrix_translate(Matrix4f *out, float x, float y, float z);

void matrix_rotate_x(Matrix4f *out, float angle);
void matrix_rotate_y(Matrix4f *out, float angle);
void matrix_rotate_z(Matrix4f *out, float angle);

void matrix_perspective(Matrix4f *out, float fov, float aspect, float near, float far);
void matrix_screen_space_transform(Matrix4f *out, float halfWidth, float halfHeight);


#define LOAD_MATRIX(m, m00, m01, m02, m03, m10, m11, m12, m13, m20, m21, m22, m23, m30, m31, m32, m33)  \
    (m)[0][0] = (m00); (m)[0][1] = (m01); (m)[0][2] = (m02); (m)[0][3] = (m03); \
    (m)[1][0] = (m10); (m)[1][1] = (m11); (m)[1][2] = (m12); (m)[1][3] = (m13); \
    (m)[2][0] = (m20); (m)[2][1] = (m21); (m)[2][2] = (m22); (m)[2][3] = (m23); \
    (m)[3][0] = (m30); (m)[3][1] = (m31); (m)[3][2] = (m32); (m)[3][3] = (m33)
