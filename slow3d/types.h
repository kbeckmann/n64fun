// Copyright (c) 2021 Konrad Beckmann
// SPDX-License-Identifier: MIT

#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    float m[4][4];
} Matrix4f;

typedef struct {
    float x;
    float y;
    float z;
    float w;
} Vector4f;

typedef struct {
    Vector4f pos;
    uint32_t col;
} Vertex;
