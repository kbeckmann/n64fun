#include <libdragon.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "crc32.h"

#include "gw_lcd.h"

#include <odroid_system.h>

/* G&W system support */
#include "gw_system.h"

/* access to internals for debug purpose */
#include "sm510.h"

#define APP_ID 20

#define WIDTH  320
#define HEIGHT 240
#define BPP      2

volatile int gTicks; /* incremented every vblank */
odroid_gamepad_state_t joystick = {0};

void odroid_display_force_refresh(void)
{
    // forceVideoRefresh = true;
}


display_context_t lockVideo(int wait)
{
  display_context_t dc;

  if (wait)
    while (!(dc = display_lock()))
      ;
  else
    dc = display_lock();
  return dc;
}

void unlockVideo(display_context_t dc)
{
  if (dc)
    display_show(dc);
}

void printText(display_context_t dc, char *msg, int x, int y)
{
    if (dc)
        graphics_draw_text(dc, x*8, y*8, msg);
}


/**
 * @brief Grab the texture buffer given a display context
 *
 * @param[in] x
 *            The display context returned from #display_lock
 *
 * @return A pointer to the drawing surface for that display context.
 */
#define __get_buffer( x ) __safe_buffer[(x)-1]
extern void *__safe_buffer[3];


void pcm_submit(void)
{

}


/* vblank callback */
void vblCallback(void) {
    gTicks++;
}

/* initialize console hardware */
void init_n64(void)
{
  debug_init(DEBUG_FEATURE_LOG_ISVIEWER);

  // REALLY ugly but works.
  stdout = stderr;

  /* enable interrupts (on the CPU) */
  init_interrupts();

  /* Initialize peripherals */
  display_init(RESOLUTION_320x240, DEPTH_16_BPP, 2, GAMMA_NONE,
               ANTIALIAS_RESAMPLE);
  set_VI_interrupt(1, 590);

  register_VI_handler(vblCallback);

  controller_init();
}

/* callback to get buttons state */
unsigned int gw_get_buttons()
{
    unsigned int hw_buttons = 0;
    hw_buttons |= joystick.values[ODROID_INPUT_LEFT];
    hw_buttons |= joystick.values[ODROID_INPUT_UP] << 1;
    hw_buttons |= joystick.values[ODROID_INPUT_RIGHT] << 2;
    hw_buttons |= joystick.values[ODROID_INPUT_DOWN] << 3;
    hw_buttons |= joystick.values[ODROID_INPUT_A] << 4;
    hw_buttons |= joystick.values[ODROID_INPUT_B] << 5;
    hw_buttons |= joystick.values[ODROID_INPUT_SELECT] << 6;
    hw_buttons |= joystick.values[ODROID_INPUT_START] << 7;
    hw_buttons |= joystick.values[ODROID_INPUT_VOLUME] << 8;
    hw_buttons |= joystick.values[ODROID_INPUT_POWER] << 9;
    return hw_buttons;
}

uint32_t __bswapsi2(uint32_t u)
{
  return ((((u)&0xff000000) >> 24) |
          (((u)&0x00ff0000) >> 8)  |
          (((u)&0x0000ff00) << 8)  |
          (((u)&0x000000ff) << 24));
}



int main(int argc, char *argv[])
{
    init_n64();

    /*** load ROM  */
    gw_system_romload();

    /*** Clear audio buffer */
    // gw_sound_init();
    // printf("Sound initialized\n");

    /*** Configure the emulated system */
    gw_system_config();
    printf("G&W configured\n");

    /*** Start and Reset the emulated system */
    gw_system_start();
    printf("G&W start\n");

    gw_system_reset();
    printf("G&W reset\n");

    /*** Main emulator loop */
    printf("Main emulator loop start\n");

    /* check if we to have to load state */
    // if (load_state != 0)
    //     gw_system_LoadState(NULL);

    while (true)
    {
        odroid_input_read_gamepad(&joystick);

        /* Emulate and Blit */
        // Call the emulator function with number of clock cycles
        // to execute on the emulated device
        gw_system_run(GW_SYSTEM_CYCLES);

        // Draw into buffer
        display_context_t _dc = lockVideo(1);
        uint16_t *dest = (uint16_t *) __get_buffer(_dc);

        if (dest == NULL) {
            printf("dest == NULL!\n");
        } else {
            gw_system_blit(dest);
        }

        unlockVideo(_dc);

        /****************************************************************************/

        /* copy audio samples for DMA */
        // gw_sound_submit();

    } // end of loop

    return 0;
}
