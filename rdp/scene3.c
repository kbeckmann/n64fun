// Copyright (c) 2021 Konrad Beckmann
// SPDX-License-Identifier: MIT
// http://arsantica-online.com/st-niccc-competition/
// Implementation inspired by http://luis.net/projects/svg/st-niccc/js/svg.js

#include "common.h"

extern uint32_t __width;
extern uint32_t __height;

void scene3(display_context_t disp, uint32_t t[8])
{
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

    rdp_sync(SYNC_PIPE);
    rdp_set_default_clipping();
    rdp_attach_display(disp);

    int ret = render_niccc(&scene_data, __width, __height);

    if (ret == 1) {
        scene_data = scene_data_start;
    }

    t[3] = TICKS_READ();

    rdp_detach_display();

    t[4] = t[5] = t[6] = t[7] = TICKS_READ();
}
