// Copyright (c) 2021 Konrad Beckmann
// SPDX-License-Identifier: MIT
// Based on https://github.com/DragonMinded/libdragon/blob/trunk/examples/vtest/vtest.c

#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <stdint.h>
#include <libdragon.h>

typedef struct PI_regs_s {
	volatile void *ram_address;
	uint32_t pi_address;
	uint32_t read_length;
	uint32_t write_length;
} PI_regs_t;
static volatile PI_regs_t *const PI_regs = (PI_regs_t *) 0xA4600000;

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

static void pi_write_raw(const void *src, uint32_t base, uint32_t offset, uint32_t len)
{
	assert(src != NULL);

	disable_interrupts();
	dma_wait();

	MEMORY_BARRIER();
	PI_regs->ram_address = UncachedAddr(src);
	MEMORY_BARRIER();
	PI_regs->pi_address = offset | base;
	MEMORY_BARRIER();
	PI_regs->read_length = len - 1;
	MEMORY_BARRIER();

	enable_interrupts();
	dma_wait();
}

static void pi_write_u32(const uint32_t value, uint32_t base, uint32_t offset)
{
	uint32_t buf[] = { value };

	data_cache_hit_writeback_invalidate(buf, sizeof(buf));
	pi_write_raw(buf, base, offset, sizeof(buf));
}

/* main code entry point */
int main(void)
{
    display_context_t _dc;
    char temp[128];
    unsigned short buttons, previous = 0;

    init_n64();

    int frames = 0;
    int increment = 1;
    while (1) {
        unsigned int bgcolor;
        unsigned int fgcolor;

        _dc = lockVideo(1);

        // Get buttons after locking to get lower latency
        buttons = getButtons(0);


        if (A_BUTTON(buttons ^ previous)) {
            if (A_BUTTON(buttons)) {
                // A is pressed. Turn off LED and stop the counter.
                pi_write_u32(0, PC64_CIBASE_ADDRESS_START, PC64_REGISTER_LED);
                increment = 0;
            } else {
                pi_write_u32(1, PC64_CIBASE_ADDRESS_START, PC64_REGISTER_LED);
                increment = 1;
            }
        }

        frames += increment;

        if (B_BUTTON(buttons)) {
            gTicks = 0;
            frames = 0;
        }

        // Gray background
        bgcolor = graphics_make_color(0x40, 0x40, 0x40, 0xFF);
        graphics_fill_screen(_dc, bgcolor);

        // White text
        fgcolor = graphics_make_color(0xFF, 0xFF, 0xFF, 0xFF);
        graphics_set_color(fgcolor, bgcolor);

        printText(_dc, "Latency Test", WIDTH/16 - 10, 3);

        sprintf(temp, "gTicks: %d", gTicks);
        printText(_dc, temp, WIDTH/16 - 3, 7);

        sprintf(temp, "frames: %d", frames);
        printText(_dc, temp, WIDTH/16 - 3, 9);

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

        previous = buttons;
    }

    return 0;
}
