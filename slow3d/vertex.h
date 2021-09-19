// Copyright (c) 2021 Konrad Beckmann
// SPDX-License-Identifier: MIT

#pragma once

#include "types.h"

void vertex_init(Vertex *out, Vector4f pos);
void vertex_transform(const Matrix4f *m, const Vertex *in, Vertex *out);
void vertex_perspective_divide(Vertex *io);
float vertex_triangle_area2(const Vertex *a, const Vertex *b, const Vertex *c);
