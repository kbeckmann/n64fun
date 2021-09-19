// Copyright (c) 2021 Konrad Beckmann
// SPDX-License-Identifier: MIT

#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <stdint.h>
#include <libdragon.h>

void scene0(display_context_t disp, uint32_t t[8])
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
    rdp_draw_filled_rectangle(0, 0, 320, 240);
    t[3] = TICKS_READ();
    t[4] = TICKS_READ();
    t[5] = TICKS_READ();
    t[6] = TICKS_READ();

    rdp_detach_display();
    t[7] = TICKS_READ();
}
