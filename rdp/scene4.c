// Copyright (c) 2021 Konrad Beckmann
// SPDX-License-Identifier: MIT

#include "common.h"

extern uint32_t __width;
extern uint32_t __height;
extern uint32_t __bitdepth;


void draw_buf_stride(uint16_t *dest, uint16_t *source, int x, int y, int width, int height, int stride)
{
    for (int row = 0; row < height; row++) {
        uint16_t *d = &dest[(row + y) * stride + x];
        uint16_t *s = &source[row * width];
        memcpy(d, s, width * __bitdepth);
        // dma_read_any(d, s, width * __bitdepth);
    }
}

void scene4(display_context_t disp, uint32_t t[8])
{
    if (__width != 640 || __height != 240 || __bitdepth != 2) {
        // unsupported resolution
        return;
    }

    static int initialized;
    static uint8_t *scene_data_start;
    static uint8_t *scene_data;

    if (!initialized) {
        initialized = 1;
        scene_data_start = niccc_get_data();
        scene_data = scene_data_start;
    }

    struct controller_data keys = get_keys_pressed();
    if (keys.c[0].start) {
        // Reset when pressing start
        scene_data = scene_data_start;
    }

    t[1] = TICKS_READ();

    // Render into a 320 x 120 buffer
    #define buf_w (640 / 2)
    #define buf_h (240 / 2)
    static uint16_t off_buf[buf_w * buf_h] __attribute__((aligned(64)));

    rdp_sync(SYNC_PIPE);
    rdp_set_clipping( 0, 0, buf_w, buf_h );
    rdp_attach_buffer(off_buf, buf_w);
    int ret = render_niccc(&scene_data, buf_w, buf_h);

    if (ret == 1) {
        scene_data = scene_data_start;
    }

    t[2] = TICKS_READ();

    uint16_t *fb = __get_buffer(disp);

    // TODO: Getting some weird artifacts. Why? Sync doesn't help.
    // data_cache_hit_writeback_invalidate(off_buf, sizeof(off_buf));

    // Draw 4x copies (24ms)
    draw_buf_stride(fb, off_buf,     0, 0,     buf_w, buf_h, __width);
    draw_buf_stride(fb, off_buf, buf_w, 0,     buf_w, buf_h, __width);
    draw_buf_stride(fb, off_buf,     0, buf_h, buf_w, buf_h, __width);
    draw_buf_stride(fb, off_buf, buf_w, buf_h, buf_w, buf_h, __width);

    t[3] = TICKS_READ();
    t[4] = t[5] = t[6] = t[7] = TICKS_READ();
}
