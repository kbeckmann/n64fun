// Copyright (c) 2021 Konrad Beckmann
// SPDX-License-Identifier: MIT

#include "common.h"

void scene2(display_context_t disp, uint32_t t[8])
{
    // Just clears the screen, nothing else

    rdp_sync(SYNC_PIPE);
    t[1] = TICKS_READ();

    rdp_set_default_clipping();
    rdp_attach_display(disp);
    rdp_sync(SYNC_PIPE);
    t[2] = TICKS_READ();

    rdp_enable_primitive_fill();
    rdp_set_primitive_color(0x00000000);
    rdp_draw_filled_rectangle(0, 0, __width, __height);
    t[3] = TICKS_READ();


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

    // matrix_translate(&translation, 0.0f, 0.0f, 3.0f);
    matrix_translate(&translation, 0.0f, -2.0f, 6.0f);
    // matrix_translate(&translation, 0.0f, 0.0f, g_frame / 10.0f);

    // matrix_rotate_y(&rotation, 0);
    matrix_rotate_y(&rotation, g_frame / 100.0f);

    matrix_mul(&translation, &rotation, &temp);
    matrix_mul(&projection, &temp, &transform);

    matrix_screen_space_transform(&screenSpaceTransform, __width / 2.0f, __height / 2.0f);

    #define CREATE_V(_x, _y, _z, _col) {.pos = {.x = (_x), .y = (_y), .z = (_z), .w = 1}, .col = (_col)}

    Vertex cube[][4] = {
        { // front, green
            CREATE_V(-1,  -1, -1, 0x00ff00ff),
            CREATE_V(-1,   1, -1, 0x00ff00ff),
            CREATE_V( 1,   1, -1, 0x00ff00ff),
            CREATE_V( 1,  -1, -1, 0x00ff00ff),
        },
        { // left, orange
            CREATE_V(-1, -1,  1, 0xffa500ff),
            CREATE_V(-1,  1,  1, 0xffa500ff),
            CREATE_V(-1,  1, -1, 0xffa500ff),
            CREATE_V(-1, -1, -1, 0xffa500ff),
        },
        { // right, red
            CREATE_V(1, -1, -1, 0xff0000ff),
            CREATE_V(1,  1, -1, 0xff0000ff),
            CREATE_V(1,  1,  1, 0xff0000ff),
            CREATE_V(1, -1,  1, 0xff0000ff),
        },
        { // back, blue
            CREATE_V( 1, -1, 1, 0x0000ffff),
            CREATE_V(-1, -1, 1, 0x0000ffff),
            CREATE_V(-1,  1, 1, 0x0000ffff),
            CREATE_V( 1,  1, 1, 0x0000ffff),
        },
        { // top, white
            CREATE_V(-1, 1, -1, 0xffffffff),
            CREATE_V(-1, 1,  1, 0xffffffff),
            CREATE_V( 1, 1,  1, 0xffffffff),
            CREATE_V( 1, 1, -1, 0xffffffff),
        },
        { // bottom, yellow
            CREATE_V(-1, -1,  1, 0xffff00ff),
            CREATE_V(-1, -1, -1, 0xffff00ff),
            CREATE_V( 1, -1, -1, 0xffff00ff),
            CREATE_V( 1, -1,  1, 0xffff00ff),
        },
    };

    t[4] = TICKS_READ();

    // TODO: Add guards to protect against FPU exceptions
    rdp_enable_blend_fill();
    uint32_t last_color = 0;
    for (int i = 0; i < ARRAY_SIZE(cube); i++) {
        Vertex *quad = &cube[i];
        for (int j = 0; j < 4; j++) {
            vertex_transform(&transform, &quad[j], &quad[j]);
            vertex_transform(&screenSpaceTransform, &quad[j], &quad[j]);
            vertex_perspective_divide(&quad[j]);
        }

        // Draw polygon as a triangle fan with a root at index 0.
        uint32_t color = quad[0].col;
        if (color != last_color) {
            rdp_sync(SYNC_PIPE);
            rdp_set_blend_color(quad[0].col);
            rdp_set_blend_color(color);
            last_color = color;
        }
        for (int i = 2; i < 4; i++) {
            rdp_draw_filled_triangle(quad[    0].pos.x, quad[    0].pos.y,
                                     quad[i - 1].pos.x, quad[i - 1].pos.y,
                                     quad[    i].pos.x, quad[    i].pos.y);
        }
    }

    t[5] = TICKS_READ();
    t[6] = TICKS_READ();

    rdp_detach_display();
    t[7] = TICKS_READ();
}
