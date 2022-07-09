// Copyright (c) 2021 Konrad Beckmann
// SPDX-License-Identifier: MIT

#include "common.h"

uint32_t g_frame = 0;

void printText(display_context_t dc, char *msg, int x, int y)
{
    if (dc)
        graphics_draw_text(dc, x*8, y*8, msg);
}

static void vblCallback(void)
{
    g_frame++;
}

scene_t *scenes[] = {
    &scene3,
    &scene2,
    &scene1,
};

int main(void)
{
    char temp[128];
    uint32_t prev_buttons = 0;
    uint32_t t_end1 = 0;
    uint32_t t_end2 = 0;
    uint32_t t_delta, t_delta_ms;
    uint32_t scene = 0;
    uint32_t cycle = 0;
    uint32_t print_stats = 0;

    resolution_t resolution = RESOLUTION_320x240;
    // resolution_t resolution = RESOLUTION_640x240;
    bitdepth_t bitdepth = DEPTH_16_BPP;
    // bitdepth_t bitdepth = DEPTH_32_BPP;
    // antialias_t antialias = ANTIALIAS_OFF;
    antialias_t antialias = ANTIALIAS_RESAMPLE;

    /* enable interrupts (on the CPU) */
    // init_interrupts();
    debug_init_isviewer();

    /* Initialize peripherals */
    display_init(resolution, bitdepth, 2, GAMMA_NONE, antialias);
    set_VI_interrupt(1, 590);

    dfs_init(DFS_DEFAULT_LOCATION);
    rdp_init();
    controller_init();
    register_VI_handler(vblCallback);

    /* Main loop test */
    while(1)
    {
        display_context_t disp = 0;

        /* Grab a render buffer */
        while(!(disp = display_lock()));

        uint32_t t[8];
        static uint32_t t0_old;
        t[0] = TICKS_READ();
        uint32_t t_frame = t[0] - t0_old;
        t0_old = t[0];

        /* Set the text output color */
        graphics_set_color(0x0, 0xFFFFFFFF);

        // Draw active scene
        scenes[scene](disp, t);

        // Cycle through the scenes automatically
        if (cycle && (g_frame % 100 == 0)) {
            scene = (scene + 1) % ARRAY_SIZE(scenes);
        }

        if (print_stats) {
            sprintf(temp, "frame %ld", g_frame);
            printText(disp, temp, 15, 0);

            t_delta_ms = t_frame / (TICKS_PER_SECOND / 1000);

            sprintf(temp, "t_frame=%ld (%ld ms)", t_frame, t_delta_ms);
            printText(disp, temp, 15, 5);

            t_delta = t[7] - t[0];
            t_delta_ms = t_delta / (TICKS_PER_SECOND / 1000);

            sprintf(temp, "t7-t0=%ld (%ld ms)", t_delta, t_delta_ms);
            printText(disp, temp, 15, 7);

            t_delta = t_end2 - t_end1;
            t_delta_ms = t_delta / (TICKS_PER_SECOND / 1000);

            sprintf(temp, "t_end=%ld (%ld ms)", t_delta, t_delta_ms);
            printText(disp, temp, 15, 9);

            for (int i = 0; i < 7; i++) {
                t_delta = t[i + 1] - t[i];
                t_delta_ms = t_delta / (TICKS_PER_SECOND / 1000);

                sprintf(temp, "t%d-t%d=%ld (%ld ms)", i + 1, i, t_delta, t_delta_ms);
                printText(disp, temp, 15, 11 + 2*i);
            }

            sprintf(temp, "%ld x %ld x %ld x AA=%d", __width, __height, __bitdepth, antialias);
            printText(disp, temp, 15, 25);
        }

        /* Force backbuffer flip */
        display_show(disp);

        t_end1 = t[7];
        t_end2 = TICKS_READ();

        /* Do we need to switch video displays? */
        controller_scan();
        struct controller_data keys = get_keys_down();


        if (prev_buttons != keys.c[0].data) {
            if (keys.c[0].A) {
                scene = (scene + 1) % ARRAY_SIZE(scenes);
            }
            if (keys.c[0].B) {
                cycle = !cycle;
            }
            if (keys.c[0].Z) {
                print_stats = !print_stats;
            }
            if (keys.c[0].up) {
                resolution = (resolution + 1) % (RESOLUTION_640x240 + 1);
                display_close();
                display_init(resolution, bitdepth, 2, GAMMA_NONE, antialias);
                set_VI_interrupt(1, 590);
            }
            if (keys.c[0].down) {
                antialias = (antialias + 1) % (ANTIALIAS_RESAMPLE_FETCH_ALWAYS + 1);
                display_close();
                display_init(resolution, bitdepth, 2, GAMMA_NONE, antialias);
                set_VI_interrupt(1, 590);
            }
        }

        prev_buttons = keys.c[0].data;
    }
}
