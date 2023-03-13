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
    Vector4f normal;
    union {
        struct {
            uint8_t r;
            uint8_t g;
            uint8_t b;
            uint8_t a;
        };
        uint32_t rgba;
    } col;
} Vertex;
