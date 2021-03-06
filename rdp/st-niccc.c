// Copyright (c) 2021 Konrad Beckmann
// SPDX-License-Identifier: MIT
// http://arsantica-online.com/st-niccc-competition/
// Implementation inspired by http://luis.net/projects/svg/st-niccc/js/svg.js

#include "common.h"

static inline uint32_t bit_test(uint32_t num, uint32_t bit)
{
    return (num & (1 << bit));
}

static inline uint8_t read_u8(uint8_t **p)
{
    uint8_t ret = (*p)[0];

    *p += 1;

    return ret;
}

static inline uint16_t read_u16(uint8_t **p)
{
    uint16_t ret = ((*p)[0] << 8) | (*p)[1];

    *p += 2;

    return ret;
}

static uint32_t atari_to_8888(uint16_t col)
{
    // from 00000RRR0GGG0BBB
    // to   R8 G8 B8 A8

    uint8_t b3 = ((col & 0x007)     ) & 0xff;
    uint8_t g3 = ((col & 0x070) >> 4) & 0xff;
    uint8_t r3 = ((col & 0x700) >> 8) & 0xff;

    uint32_t b = b3 << (5 + 8);
    uint32_t g = g3 << (5 + 8 + 8);
    uint32_t r = r3 << (5 + 8 + 8 + 8);

    return (r | g | b | 0xFF);
}

uint8_t* niccc_get_data(void)
{
    static int initialized;
    static uint8_t *scene_data;

    if (!initialized) {
        initialized = 1;

        // Load it all into RAM, we have space
        int fp = dfs_open("/st-niccc.bin");
        scene_data = malloc(dfs_size(fp) + 0x10000);
        scene_data = (uint8_t *) (((uint32_t) scene_data + 0x10000) & ~0xffff); // Align with 64kB
        dfs_read(scene_data, 1, dfs_size(fp), fp);
        dfs_close(fp);
    }

    return scene_data;
}

uint32_t render_niccc(uint8_t **scene_data, uint32_t width, uint32_t height)
{
    uint32_t ret = 0;

    static uint32_t colorArray[16];
    static uint16_t vertArray[256];

    float sx = 1.0f;
    float sy = 1.0f;

    sx = ((float) width) / 256;
    sy = ((float) height) / 200;

    rdp_sync(SYNC_PIPE);
    rdp_enable_primitive_fill();
    rdp_set_primitive_color(0x00000000);
    rdp_draw_filled_rectangle(0, 0, width, height);

    rdp_enable_blend_fill();
    rdp_sync(SYNC_PIPE);

    // Parse ST-NICCC data
    uint8_t flags = read_u8(scene_data);

    // Bit 0: Frame needs to clear the screen.
    // int flag_clearScreen = bit_test(flags, 0);

    // Bit 1: Frame contains palette data.
    int flag_hasPalette = bit_test(flags, 1);

    // Bit 2: Frame is stored in indexed mode.
    int flag_isIndexed = bit_test(flags, 2);

    // If frame contains palette data
    if (flag_hasPalette) {
        uint16_t bitMask = read_u16(scene_data);
        for (int x = 0; x < 16; x++) {
            if (bit_test(bitMask, 15 - x)) {
                uint16_t col = read_u16(scene_data);
                colorArray[x] = atari_to_8888(col);
            }
        }
    }

    if (flag_isIndexed) {
        // 1 byte Number of vertices (0-255)
        uint8_t numberVertices = read_u8(scene_data);
        for (int x = 0; x < numberVertices; x++) {
            // 1 byte X-position, 1 byte Y-position
            vertArray[x] = read_u16(scene_data);
        }
    }

    uint32_t last_color = 0;

    // hi-nibble - 4 bits color-index
    // lo-nibble - 4 bits number of polygon vertices
    int done = 0;
    while (!done) {
        uint8_t bits = read_u8(scene_data);
        switch (bits) {
            case 0xFE: // End of frame and the stream skips to the next 64KB block
                *scene_data = (uint8_t *) ((((uint32_t) *scene_data) + 0x10000) & ~0xFFFF);
            case 0xFF: // End of frame
                done = 1;
                break;
            case 0xFD: // End of stream
                ret = 1;
                done = 1;
                break;
            default:
            {
                float points_x[16];
                float points_y[16];
                uint32_t color = colorArray[(bits & 0xF0) >> 4];

                for (int ii = 0; ii < (bits & 0xF); ii++) {
                    uint16_t point_xy = (flag_isIndexed ? vertArray[read_u8(scene_data)] : read_u16(scene_data));
                    points_x[ii] = (point_xy >> 8)   * sx;
                    points_y[ii] = (point_xy & 0xff) * sy;
                }

                // A SYNC_PIPE has to be issued if the attributes change and are
                // used by the last two primitives.
                if (color != last_color) {
                    rdp_sync(SYNC_PIPE);
                    rdp_set_blend_color(color);
                    last_color = color;
                }

                // Draw polygon as a triangle fan with a root at index 0.
                // Might have to tesselate?
                for (int i = 2; i < (bits & 0xF); i++) {
                    rdp_draw_filled_triangle(points_x[    0], points_y[    0],
                                             points_x[i - 1], points_y[i - 1],
                                             points_x[    i], points_y[    i]);
                }
            }
        }
    }

    rdp_detach_display();

    return ret;
}
