#pragma once

#include <stdio.h>
#include <malloc.h>
#include <math.h>
#include <string.h>
#include <stdint.h>
#include <libdragon.h>
#include <cop1.h>

// Access to the raw frame buffers from libdragon
#define __get_buffer(x) __safe_buffer[(x) - 1]
extern void *__safe_buffer[3];

extern uint32_t __width;
extern uint32_t __height;
extern uint32_t __bitdepth;

extern uint32_t g_frame;

void scene0(display_context_t disp, uint32_t t[8]);
void scene1(display_context_t disp, uint32_t t[8]);
void scene2(display_context_t disp, uint32_t t[8]);
void scene3(display_context_t disp, uint32_t t[8]);
void scene4(display_context_t disp, uint32_t t[8]);
void scene5(display_context_t disp, uint32_t t[8]);

// st-niccc.c
uint8_t* niccc_get_data(void);
uint32_t render_niccc(uint8_t **scene_data, uint32_t width, uint32_t height);

