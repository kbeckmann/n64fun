#include <libdragon.h>

#include <odroid_system.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <odroid_system.h>

#include "porting.h"
#include "crc32.h"

#include "gw_lcd.h"
#include "gnuboy/loader.h"
#include "gnuboy/hw.h"
#include "gnuboy/lcd.h"
#include "gnuboy/cpu.h"
#include "gnuboy/mem.h"
#include "gnuboy/sound.h"
#include "gnuboy/regs.h"
#include "gnuboy/rtc.h"
#include "gnuboy/defs.h"

#define APP_ID 20

#define NVS_KEY_SAVE_SRAM "sram"

#define WIDTH  GB_WIDTH
#define HEIGHT GB_HEIGHT
#define BPP      2
#define SCALE    4

#define AUDIO_SAMPLE_RATE   (48000)
#define AUDIO_BUFFER_LENGTH (AUDIO_SAMPLE_RATE / 60)

// Use 60Hz for GB
#define AUDIO_BUFFER_LENGTH_GB (AUDIO_SAMPLE_RATE / 60)
#define AUDIO_BUFFER_LENGTH_DMA_GB ((2 * AUDIO_SAMPLE_RATE) / 60)

static odroid_video_frame_t update1 = {GB_WIDTH, GB_HEIGHT, GB_WIDTH * 2, 2, 0xFF, -1, NULL, NULL, 0, {}};
static odroid_video_frame_t update2 = {GB_WIDTH, GB_HEIGHT, GB_WIDTH * 2, 2, 0xFF, -1, NULL, NULL, 0, {}};
static odroid_video_frame_t *currentUpdate = &update1;

static bool fullFrame = false;
static uint skipFrames = 0;

static bool saveSRAM = false;

// 3 pages
uint8_t state_save_buffer[192 * 1024];

uint16_t fb_data[WIDTH * HEIGHT * BPP];

extern unsigned char cart_rom[];
extern unsigned int cart_rom_len;

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


__attribute__((optimize("unroll-loops")))
static inline void blit(void) {
    const int w1 = GB_WIDTH;
    const int w2 = 320;
    const int h1 = GB_HEIGHT;
    const int hpad = 27;
    int index = 0;

    display_context_t _dc = lockVideo(1);
    uint16_t *n64_fb = (uint16_t *) __get_buffer(_dc);

    for (int y = 0; y < h1; y++) {
        uint16_t *dest_row = &n64_fb[y * w2 + hpad];
        for (int x = 0; x < w1; x++) {
            uint16_t c = 
              ((fb_data[index] & 0b1111111111000000) | 1) | // Reuse R and G, set alpha = 1
              ((fb_data[index] << 1) & 0b111110);           // Shift and mask out B

            dest_row[x] = c;

            index++;
        }
    }

    unlockVideo(_dc);
}

int init_window(int width, int height)
{
  

    return 0;
}

static void netplay_callback(netplay_event_t event, void *arg)
{
    // Where we're going we don't need netplay!
}

static bool SaveState(char *pathName)
{
    return 0;
}

static bool LoadState(char *pathName)
{
    return true;
}

void pcm_submit(void)
{

}

volatile int gTicks; /* incremented every vblank */

/* vblank callback */
void vblCallback(void) {
    gTicks++;
}

/* initialize console hardware */
void init_n64(void)
{
  /* enable interrupts (on the CPU) */
  init_interrupts();

  /* Initialize peripherals */
  display_init(RESOLUTION_320x240, DEPTH_16_BPP, 2, GAMMA_NONE,
               ANTIALIAS_RESAMPLE);
  set_VI_interrupt(1, 590);

  register_VI_handler(vblCallback);

  controller_init();
}

void init(void)
{
    init_n64();

    odroid_system_init(APP_ID, AUDIO_SAMPLE_RATE);
    odroid_system_emu_init(&LoadState, &SaveState, &netplay_callback);

    // Hack: Use the same buffer twice
    update1.buffer = fb_data;
    update2.buffer = fb_data;

    //saveSRAM = odroid_settings_app_int32_get(NVS_KEY_SAVE_SRAM, 0);
    saveSRAM = false;

    // Load ROM
    loader_init(NULL);

    // RTC
    memset(&rtc, 0, sizeof(rtc));

    // Video
    memset(fb_data, 0, sizeof(fb_data));
    memset(&fb, 0, sizeof(fb));
    fb.w = GB_WIDTH;
    fb.h = GB_HEIGHT;
    fb.format = GB_PIXEL_565_LE;
    fb.pitch = update1.stride;
    fb.ptr = currentUpdate->buffer;
    fb.enabled = 1;
    fb.blit_func = &blit;

    emu_init();

    //pal_set_dmg(odroid_settings_Palette_get());
    // pal_set_dmg(2);
    pal_set_dmg(2);
}


int main(int argc, char *argv[])
{
    init_window(WIDTH, HEIGHT);

    init();
    odroid_gamepad_state_t joystick = {0};

    while (true)
    {
        odroid_input_read_gamepad(&joystick);

        uint startTime = get_elapsed_time();
        bool drawFrame = !skipFrames;

        pad_set(PAD_UP, joystick.values[ODROID_INPUT_UP]);
        pad_set(PAD_RIGHT, joystick.values[ODROID_INPUT_RIGHT]);
        pad_set(PAD_DOWN, joystick.values[ODROID_INPUT_DOWN]);
        pad_set(PAD_LEFT, joystick.values[ODROID_INPUT_LEFT]);
        pad_set(PAD_SELECT, joystick.values[ODROID_INPUT_SELECT]);
        pad_set(PAD_START, joystick.values[ODROID_INPUT_START]);
        pad_set(PAD_A, joystick.values[ODROID_INPUT_A]);
        pad_set(PAD_B, joystick.values[ODROID_INPUT_B]);

        emu_run(drawFrame);

        // Tick before submitting audio/syncing
        odroid_system_tick(!drawFrame, fullFrame, get_elapsed_time_since(startTime));
    }

    return 0;
}
