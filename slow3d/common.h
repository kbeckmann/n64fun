// Copyright (c) 2021 Konrad Beckmann
// SPDX-License-Identifier: MIT

#pragma once

#include <stdio.h>
#include <malloc.h>
#include <math.h>
#include <string.h>
#include <stdint.h>
#include <libdragon.h>
#include <cop1.h>

#include "types.h"
#include "matrix.h"
#include "vector.h"
#include "vertex.h"

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

// Access to the raw frame buffers from libdragon
#define __get_buffer(x) __safe_buffer[(x) - 1]
extern void *__safe_buffer[3];

// disp: Display context
// t[8]: Profiling timestamps
typedef void (scene_t)(display_context_t disp, uint32_t t[8]);

extern uint32_t __width;
extern uint32_t __height;
extern uint32_t __bitdepth;

extern uint32_t g_frame;

void printText(display_context_t dc, char *msg, int x, int y);

void scene1(display_context_t disp, uint32_t t[8]);
void scene2(display_context_t disp, uint32_t t[8]);


