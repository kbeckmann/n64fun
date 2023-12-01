// Copyright (c) 2023 Konrad Beckmann
// SPDX-License-Identifier: MIT
// Based on https://github.com/DragonMinded/libdragon/blob/trunk/examples/vtest/vtest.c

#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
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

// PicoCart64 Address space

// [READ/WRITE]: Command address space. See register definitions below for details.
#define PC64_CIBASE_ADDRESS_START  0x83000000
#define PC64_CIBASE_ADDRESS_LENGTH 0x00001000
#define PC64_CIBASE_ADDRESS_END    (PC64_CIBASE_ADDRESS_START + PC64_CIBASE_ADDRESS_LENGTH - 1)

// [WRITE]: Write control LED
//     Bit  0: Red / The single LED on the Pico
//     Bit  1: Green
//     Bit  2: Blue
#define PC64_REGISTER_LED          0x0000000C


#define WIDTH 320

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

#define AUDIO_BUFFER_SIZE (48000)

typedef struct audio_sample {
    int16_t channels[2];
} audio_sample_t;

audio_sample_t audio_buffer_a[AUDIO_BUFFER_SIZE];

void generate(int gain)
{
    // Generate sine
    #define SAMPLE_RATE 48000
    #define FREQUENCY1 375
    #define FREQUENCY2 (375 * 1.5)
    // #define M_PI 3.1415926535

    float gainf = 32768.0f / gain;

    // Generate 1 second of samples. This *will* make a click every 1 full second, but at least it's known.
    for (int i = 0; i < AUDIO_BUFFER_SIZE; i++) {
        double time = (double)i / SAMPLE_RATE;
        int16_t sample1 = (int16_t)(gainf * sinf(2.0 * M_PI * FREQUENCY1 * time));
        int16_t sample2 = (int16_t)(gainf * sinf(2.0 * M_PI * FREQUENCY2 * time));

        audio_buffer_a[i].channels[0] = sample1;
        audio_buffer_a[i].channels[1] = sample2;
    }
}

void generate_saw(int gain)
{
    // Generate sine
    #define SAMPLE_RATE 48000
    #define FREQUENCY1 375
    #define FREQUENCY2 (375 * 1.5)
    // #define M_PI 3.1415926535

    float gainf = (float) gain;

    // Generate 1 second of samples. This *will* make a click every 1 full second, but at least it's known.
    for (int i = 0; i < AUDIO_BUFFER_SIZE; i++) {
        double time = (double)i / SAMPLE_RATE;
        int16_t sample1 = ((i * 1024) % 65536) - 32768;
        int16_t sample2 = ((i * 1536) % 65536) - 32768;

        audio_buffer_a[i].channels[0] = sample1 / gain;
        audio_buffer_a[i].channels[1] = sample2 / gain;
    }
}


void generate_pattern(void)
{
    for (int i = 0; i < AUDIO_BUFFER_SIZE; i++) {
        audio_buffer_a[i].channels[0] = 0x1234;
        audio_buffer_a[i].channels[1] = 0x5678;
    }
}

/* main code entry point */
int main(void)
{
    display_context_t _dc;
    char temp[128];
    unsigned short buttons = 0;
    unsigned short previous = 0;

    int gain = 2;

    init_n64();

    // generate(gain);
    generate_saw(gain);


    audio_init(SAMPLE_RATE, 4);
    int offset = 0;


    int frames = 0;
    int increment = 1;
    while (1) {
        unsigned int bgcolor;
        unsigned int fgcolor;

        _dc = lockVideo(1);

        // Get buttons after locking to get lower latency
        buttons = getButtons(0);

        frames += 1;

        if (B_BUTTON(buttons)) {
            gTicks = 0;
            frames = 0;
        }

        if (DU_BUTTON(buttons ^ previous) && !DU_BUTTON(buttons)) {
            if (gain < 16384) {
                gain *= 2;
                generate_saw(gain);
            }
        }

        if (DD_BUTTON(buttons ^ previous) && !DD_BUTTON(buttons)) {
            if (gain > 1) {
                gain /= 2;
                generate_saw(gain);
            }
        }

        if (DR_BUTTON(buttons ^ previous) && !DR_BUTTON(buttons)) {
            generate_pattern();
        }

        if (DL_BUTTON(buttons ^ previous) && !DL_BUTTON(buttons)) {
            generate(gain);
        }

        // Gray background
        bgcolor = graphics_make_color(0x40, 0x40, 0x40, 0xFF);
        graphics_fill_screen(_dc, bgcolor);

        // White text
        fgcolor = graphics_make_color(0xFF, 0xFF, 0xFF, 0xFF);
        graphics_set_color(fgcolor, bgcolor);

        printText(_dc, "Audio Test", WIDTH/16 - 10, 3);

        sprintf(temp, "gTicks: %d", gTicks);
        printText(_dc, temp, WIDTH/16 - 3, 7);

        sprintf(temp, "frames: %d", frames);
        printText(_dc, temp, WIDTH/16 - 3, 9);

        sprintf(temp, "audio buf: %d", audio_get_buffer_length());
        printText(_dc, temp, WIDTH/16 - 3, 11);

        sprintf(temp, "audio gain: 1/%d", gain);
        printText(_dc, temp, WIDTH/16 - 3, 12);

        // To make it extra clear if the counter is running or not,
        // show a green bar when counting, and a red when stopped.
        if (increment) {
            fgcolor = graphics_make_color(0x00, 0xFF, 0x00, 0xFF);
            graphics_draw_box(_dc, 0, 0, 25, 240, fgcolor);
        } else {
            fgcolor = graphics_make_color(0xFF, 0x00, 0x00, 0xFF);
            graphics_draw_box(_dc, 25, 0, 25, 240, fgcolor);
        }

        unlockVideo(_dc);


        if (audio_can_write()) {
            short* p = audio_write_begin();
            int samples = audio_get_buffer_length();
            memcpy(p, audio_buffer_a + offset, samples * 4);
            offset += samples;
            if (offset + samples >= AUDIO_BUFFER_SIZE) {
                offset = 0;
            }
            audio_write_end();
        }


        previous = buttons;
    }

    return 0;
}
