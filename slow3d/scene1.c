// Copyright (c) 2021 Konrad Beckmann
// SPDX-License-Identifier: MIT

#include "common.h"

void scene1(display_context_t disp, uint32_t t[8])
{
    // Just clears the screen, nothing else

    rdp_sync(SYNC_PIPE);
    t[1] = TICKS_READ();

    rdp_set_default_clipping();
    rdp_attach_display(disp);
    rdp_sync(SYNC_PIPE);
    t[2] = TICKS_READ();

    rdp_enable_primitive_fill();
    rdp_set_primitive_color(0xFFFFFFFF);
    rdp_draw_filled_rectangle(0, 0, __width, __height);
    t[3] = TICKS_READ();
    t[4] = TICKS_READ();
    t[5] = TICKS_READ();
    t[6] = TICKS_READ();


    Matrix4f projection;
    Matrix4f translation;
    Matrix4f rotation;
    Matrix4f transform;
    Matrix4f screenSpaceTransform;
    Matrix4f temp;

    float fov_degrees = 70;
    float fov = fov_degrees * (M_PI / 180.0f);
    float aspect = __width / ((float) __height);
    float near = 0.01f;
    float far = 100.0f;

    matrix_perspective(&projection, fov, aspect, near, far);

    matrix_translate(&translation, 0.0f, 0.0f, 3.0f);
    // matrix_translate(&translation, 0.0f, 0.0f, g_frame / 10.0f);

    // matrix_rotate_y(&rotation, 0);
    matrix_rotate_y(&rotation, g_frame / 10.0f);

    matrix_mul(&translation, &rotation, &temp);
    matrix_mul(&projection, &temp, &transform);

    matrix_screen_space_transform(&screenSpaceTransform, __width / 2.0f, __height / 2.0f);

    Vertex minYVert = {.pos = {
                           .x = -1,
                           .y = -1,
                           .z =  0,
                           .w =  1,
                       }};

    Vertex midYVert = {.pos = {
                           .x = 0,
                           .y = 1,
                           .z = 0,
                           .w = 1,
                       }};

    Vertex maxYVert = {.pos = {
                           .x =  1,
                           .y = -1,
                           .z =  0,
                           .w =  1,
                       }};

    vertex_transform(&transform, &minYVert, &minYVert);
    vertex_transform(&transform, &midYVert, &midYVert);
    vertex_transform(&transform, &maxYVert, &maxYVert);

    vertex_transform(&screenSpaceTransform, &minYVert, &minYVert);
    vertex_transform(&screenSpaceTransform, &midYVert, &midYVert);
    vertex_transform(&screenSpaceTransform, &maxYVert, &maxYVert);

    vertex_perspective_divide(&minYVert);
    vertex_perspective_divide(&midYVert);
    vertex_perspective_divide(&maxYVert);

    // TODO: Add guards to protect against FPU exceptions

    rdp_enable_blend_fill();
    rdp_set_blend_color(0x0000FF7F);
    rdp_draw_filled_triangle(
        minYVert.pos.x, minYVert.pos.y,
        midYVert.pos.x, midYVert.pos.y,
        maxYVert.pos.x, maxYVert.pos.y
    );



    rdp_detach_display();
    t[7] = TICKS_READ();
}
