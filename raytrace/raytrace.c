// Copyright (c) 2021 Konrad Beckmann
// SPDX-License-Identifier: MIT
// Based on https://github.com/DragonMinded/libdragon/blob/trunk/examples/vtest/vtest.c

#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <stdint.h>
#include <libdragon.h>

volatile int gTicks; /* incremented every vblank */

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
    /* enable interrupts (on the CPU) */
    init_interrupts();

    /* Initialize peripherals */
    display_init( RESOLUTION_320x240, DEPTH_16_BPP, 2, GAMMA_NONE, ANTIALIAS_RESAMPLE );

    register_VI_handler(vblCallback);

    controller_init();
}

extern void render_raytrace(void);

/* main code entry point */
int main(void)
{
    init_n64();
    render_raytrace();

    return 0;
}
