// Copyright (c) 2021 Konrad Beckmann
// SPDX-License-Identifier: MIT

#include "common.h"

void scene2(display_context_t disp, uint32_t t[8])
{
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

    matrix_translate(&translation, 0.0f, 0.0f, 4.0f);

    // Rotate the cube 45 degrees around the x and z axis, then rotate over time around the y axis.
    Matrix4f rot_x;
    Matrix4f rot_y;
    Matrix4f rot_z;
    Matrix4f tmp;
    matrix_rotate_x(&rot_x, M_PI/4);
    matrix_rotate_z(&rot_z, M_PI/4);
    matrix_rotate_y(&rot_y, g_frame / 30.0f);
    matrix_mul(&rot_z, &rot_x, &tmp);
    matrix_mul(&rot_y, &tmp, &rotation);

    matrix_mul(&translation, &rotation, &temp);
    matrix_mul(&projection, &temp, &transform);

    matrix_screen_space_transform(&screenSpaceTransform, __width / 2.0f, __height / 2.0f);

    #define CREATE_V(_x, _y, _z, _col) {.pos = {.x = (_x), .y = (_y), .z = (_z), .w = 1}, .col.rgba = (_col)}

    Vertex cube[][4] = {
        { // front, green
            CREATE_V(-1,  -1, -1, 0x00ff00ff),
            CREATE_V(-1,   1, -1, 0x00ff00ff),
            CREATE_V( 1,   1, -1, 0x00ff00ff),
            CREATE_V( 1,  -1, -1, 0x00ff00ff),
        },
        { // left, orange
            CREATE_V(-1, -1,  1, 0xff8c00ff),
            CREATE_V(-1,  1,  1, 0xff8c00ff),
            CREATE_V(-1,  1, -1, 0xff8c00ff),
            CREATE_V(-1, -1, -1, 0xff8c00ff),
        },
        { // right, red
            CREATE_V(1, -1, -1, 0xff0000ff),
            CREATE_V(1,  1, -1, 0xff0000ff),
            CREATE_V(1,  1,  1, 0xff0000ff),
            CREATE_V(1, -1,  1, 0xff0000ff),
        },
        { // back, blue
            CREATE_V( 1, -1, 1, 0x0000ffff),
            CREATE_V( 1,  1, 1, 0x0000ffff),
            CREATE_V(-1,  1, 1, 0x0000ffff),
            CREATE_V(-1, -1, 1, 0x0000ffff),
        },
        { // up, white
            CREATE_V(-1, 1, -1, 0xffffffff),
            CREATE_V(-1, 1,  1, 0xffffffff),
            CREATE_V( 1, 1,  1, 0xffffffff),
            CREATE_V( 1, 1, -1, 0xffffffff),
        },
        { // down, yellow
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
        Vertex *quad = cube[i];
        for (int j = 0; j < 4; j++) {
            vertex_transform(&transform, &quad[j], &quad[j]);
            vertex_transform(&screenSpaceTransform, &quad[j], &quad[j]);
            vertex_perspective_divide(&quad[j]);
        }

        // Back-face culling. Only draw clock-wise/right hand triangles.
        if (vertex_triangle_area2(&quad[0], &quad[1], &quad[2]) <= 0) {
            continue;
        }

        // Sync pipe and set color if it is changed
        uint32_t color = quad[0].col.rgba;
        if (color != last_color) {
            rdp_sync(SYNC_PIPE);
            rdp_set_blend_color(color);
            last_color = color;
        }

        // fprintf(stderr, "%f\t%f\n",   quad[0].pos.x, quad[0].pos.y);
        // fprintf(stderr, "%f\t%f\n",   quad[1].pos.x, quad[1].pos.y);
        // fprintf(stderr, "%f\t%f\n",   quad[2].pos.x, quad[2].pos.y);
        // fprintf(stderr, "%f\t%f\n\n", quad[3].pos.x, quad[3].pos.y);

        // Draw polygon as a triangle fan with a root at index 0.
        for (int i = 2; i < 4; i++) {
            rdp_draw_filled_triangle((int)quad[    0].pos.x, (int)quad[    0].pos.y,
                                     (int)quad[i - 1].pos.x, (int)quad[i - 1].pos.y,
                                     (int)quad[    i].pos.x, (int)quad[    i].pos.y);
        }
    }

    t[5] = TICKS_READ();
    t[6] = TICKS_READ();

    rdp_detach_display();
    t[7] = TICKS_READ();
}
