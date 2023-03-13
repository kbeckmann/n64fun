// Copyright (c) 2021 Konrad Beckmann
// SPDX-License-Identifier: MIT
// Based on https://github.com/DragonMinded/libdragon/blob/trunk/examples/vtest/vtest.c

#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <stdint.h>
#include <libdragon.h>

/* hardware definitions */
// Pad buttons
#define A_BUTTON(a)     ((a) & 0x8000)
#define B_BUTTON(a)     ((a) & 0x4000)
#define Z_BUTTON(a)     ((a) & 0x2000)
#define START_BUTTON(a) ((a) & 0x1000)

// D-Pad
#define DU_BUTTON(a)    ((a) & 0x0800)
#define DD_BUTTON(a)    ((a) & 0x0400)
#define DL_BUTTON(a)    ((a) & 0x0200)
#define DR_BUTTON(a)    ((a) & 0x0100)

// Triggers
#define TL_BUTTON(a)    ((a) & 0x0020)
#define TR_BUTTON(a)    ((a) & 0x0010)

// Yellow C buttons
#define CU_BUTTON(a)    ((a) & 0x0008)
#define CD_BUTTON(a)    ((a) & 0x0004)
#define CL_BUTTON(a)    ((a) & 0x0002)
#define CR_BUTTON(a)    ((a) & 0x0001)

#define PAD_DEADZONE     5
#define PAD_ACCELERATION 10
#define PAD_CHECK_TIME   40


unsigned short gButtons = 0;
struct controller_data gKeys;

volatile int gTicks;                    /* incremented every vblank */

/* input - do getButtons() first, then getAnalogX() and/or getAnalogY() */
unsigned short getButtons(int pad)
{
    // Read current controller status
    controller_scan();
    gKeys = get_keys_pressed();
    return (unsigned short)(gKeys.c[0].data >> 16);
}

unsigned char getAnalogX(int pad)
{
    return (unsigned char)gKeys.c[pad].x;
}

unsigned char getAnalogY(int pad)
{
    return (unsigned char)gKeys.c[pad].y;
}

display_context_t lockVideo(int wait)
{
    display_context_t dc;

    if (wait)
        while (!(dc = display_lock()));
    else
        dc = display_lock();
    return dc;
}

void unlockVideo(display_context_t dc)
{
    if (dc)
        display_show(dc);
}

/* text functions */
void drawText(display_context_t dc, char *msg, int x, int y)
{
    if (dc)
        graphics_draw_text(dc, x, y, msg);
}

void printText(display_context_t dc, char *msg, int x, int y)
{
    if (dc)
        graphics_draw_text(dc, x*8, y*8, msg);
}

/* vblank callback */
void vblCallback(void)
{
    gTicks++;
}

void delay(int cnt)
{
    int then = gTicks + cnt;
    while (then > gTicks) ;
}

/* initialize console hardware */
void init_n64(void)
{
    debug_init_isviewer();

    printf("Ohai\n");

    /* Initialize peripherals */
    display_init( RESOLUTION_320x240, DEPTH_32_BPP, 2, GAMMA_NONE, ANTIALIAS_RESAMPLE );

    register_VI_handler(vblCallback);

    controller_init();

}

/* main code entry point */
int main(void)
{
    display_context_t _dc;
    char temp[128];
    int res = 0;
    unsigned short buttons, previous = 0;

    init_n64();

    int frames = 0;
    int scene = 0;
    int vi_line = 0x200; // default from libdragon
    while (1) {
        int width[6]  = { 320, 256, 512, 640, 512, 640 };
        int height[6] = { 240, 240, 240, 240, 480, 480 };
        unsigned int color;

        _dc = lockVideo(1);
        frames++;

        color = graphics_make_color(0xFF, 0xFF, 0xFF, 0xFF);
        graphics_fill_screen(_dc, color);

        int border = 1;

        const int max_scenes = 10;
        switch (scene % max_scenes) {
        case 0:
            // Draw gradient from top to bottom
            for (int i = 0; i < height[res]; i++) {
                graphics_draw_box(_dc, 
                    0, i,
                    width[res], 1,
                    graphics_make_color(i, 0, 0, 0xFF));
            }
            break;
        case 1:
            // Draw gradient from top to bottom
            for (int i = 0; i < height[res]; i++) {
                graphics_draw_box(_dc, 
                    0, i,
                    width[res], 1,
                    graphics_make_color(0, i, 0, 0xFF));
            }
            break;
        case 2:
            // Draw gradient from top to bottom
            for (int i = 0; i < height[res]; i++) {
                graphics_draw_box(_dc, 
                    0, i,
                    width[res], 1,
                    graphics_make_color(0, 0, i, 0xFF));
            }
            break;


        case 3:
            // Draw Checkerboard all over
            for (int y = 0; y < height[res]; y++) {
                for (int x = 0; x < width[res]; x++) {
                    graphics_draw_pixel(_dc,
                        x, y,
                        ((x + y) % 2) ? graphics_make_color(0xFF, 0xFF, 0xFF, 0xFF) : graphics_make_color(0, 0, 0, 0xFF)
                    );
                }
            }
            break;
        case 4:
            // Draw Checkerboard all over, invert colors every frame
            for (int y = 0; y < height[res]; y++) {
                for (int x = 0; x < width[res]; x++) {
                    graphics_draw_pixel(_dc,
                        x, y,
                        (((x + y) % 2) == (frames % 2)) ? graphics_make_color(0xFF, 0xFF, 0xFF, 0xFF) : graphics_make_color(0, 0, 0, 0xFF)
                    );
                }
            }
            break;
        case 5:
            // Draw diagonal stripes
            for (int y = 0; y < height[res]; y++) {
                for (int x = 0; x < width[res]; x++) {
                    graphics_draw_pixel(_dc,
                        x, y,
                        ((x + y) % 16 < 8) ? graphics_make_color(0xFF, 0xFF, 0xFF, 0xFF) : graphics_make_color(0, 0, 0, 0xFF)
                    );
                }
            }
            break;


        case 6:
            // Draw border on the outer-most pixels
                border = 1;
                graphics_draw_box(_dc, 
                    border, border,
                    width[res] - 2 * border, height[res] - 2 * border,
                    graphics_make_color(0x00, 0x00, 0xFF, 0xFF));
            break;
        case 7:
            // Draw border on the outer-most pixels
                border = 2;
                graphics_draw_box(_dc, 
                    border, border,
                    width[res] - 2 * border, height[res] - 2 * border,
                    graphics_make_color(0x00, 0x00, 0xFF, 0xFF));
            break;
        case 8:
            // Draw border on the outer-most pixels
                border = 4;
                graphics_draw_box(_dc, 
                    border, border,
                    width[res] - 2 * border, height[res] - 2 * border,
                    graphics_make_color(0x00, 0x00, 0xFF, 0xFF));
            break;
        case 9:
            // Draw border on the outer-most pixels
                border = 8;
                graphics_draw_box(_dc, 
                    border, border,
                    width[res] - 2 * border, height[res] - 2 * border,
                    graphics_make_color(0x00, 0x00, 0xFF, 0xFF));
            break;
        }

        // Pink text
        graphics_set_color(graphics_make_color(0xFF, 0, 0xFF, 0xFF), 0);

        printText(_dc, "Pattern Test", width[res]/16 - 10, 3);
        switch (res) {
        case 0:
            printText(_dc, "320x240p", width[res]/16 - 3, 5);
            break;
        case 1:
            printText(_dc, "256x240p", width[res]/16 - 3, 5);
            break;
        case 2:
            printText(_dc, "512x240p", width[res]/16 - 3, 5);
            break;
        case 3:
            printText(_dc, "640x240p", width[res]/16 - 3, 5);
            break;
        case 4:
            printText(_dc, "512x480i", width[res]/16 - 3, 5);
            break;
        case 5:
            printText(_dc, "640x480i", width[res]/16 - 3, 5);
            break;
        }

        sprintf(temp, "gTicks: %d", gTicks);
        printText(_dc, temp, width[res]/16 - 3, 7);

        sprintf(temp, "frames: %d", frames);
        printText(_dc, temp, width[res]/16 - 3, 9);

        sprintf(temp, "VI_interrupt(1, %d)", vi_line);
        printText(_dc, temp, width[res]/16 - 10, 11);

        sprintf(temp, "get_tv_type(): %d", get_tv_type());
        printText(_dc, temp, width[res]/16 - 10, 13);

        unlockVideo(_dc);

        buttons = getButtons(0);

        if (A_BUTTON(buttons ^ previous)) {
            // A changed
            if (!A_BUTTON(buttons)) {
                resolution_t mode[] = {
                    RESOLUTION_320x240, // 320x240p
                    RESOLUTION_256x240, // 256x240p
                    RESOLUTION_512x240, // 512x240p
                    RESOLUTION_640x240, // 640x240p
                    // RESOLUTION_512x480, // 512x480i
                    // RESOLUTION_640x480, // 640x480i
                };
                res++;
                res %= sizeof(mode)/sizeof(mode[0]);
                display_close();
                // display_init(mode[res], DEPTH_32_BPP, 2, GAMMA_NONE, ANTIALIAS_RESAMPLE);
                display_init(mode[res], DEPTH_32_BPP, 2, GAMMA_NONE, ANTIALIAS_OFF);
                set_VI_interrupt(1, vi_line);
            }
        }

        if (B_BUTTON(buttons ^ previous) && !B_BUTTON(buttons)) {
            gTicks = 0;
            frames = 0;
        }

        if (DU_BUTTON(buttons ^ previous) && !DU_BUTTON(buttons)) {
            scene++;
        }

        if (DD_BUTTON(buttons ^ previous) && !DD_BUTTON(buttons)) {
            scene--;
        }

        if (DR_BUTTON(buttons)) {
            if (vi_line < 624) {
                vi_line++;
                set_VI_interrupt(1, vi_line);
            }
        }

        if (DL_BUTTON(buttons)) {
            if (vi_line > 3) {
                vi_line--;
                set_VI_interrupt(1, vi_line);
            }
        }

        previous = buttons;
        printf("Z");
    }

    return 0;
}
