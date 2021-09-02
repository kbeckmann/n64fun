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

#define WIDTH 320
#define HEIGHT 240
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
__attribute__((section (".itcram_hot_text")))
static inline void screen_blit_v3to5(void) {
    // static uint32_t lastFPSTime = 0;
    // static uint32_t frames = 0;
    // uint32_t currentTime = HAL_GetTick();
    // uint32_t delta = currentTime - lastFPSTime;

    // frames++;

    // if (delta >= 1000) {
    //     int fps = (10000 * frames) / delta;
    //     printf("FPS: %d.%d, frames %ld, delta %ld ms, skipped %d\n", fps / 10, fps % 10, delta, frames, common_emu_state.skipped_frames);
    //     frames = 0;
    //     common_emu_state.skipped_frames = 0;
    //     lastFPSTime = currentTime;
    // }

    display_context_t _dc = lockVideo(1);
    uint16_t *dest = (uint16_t *) __get_buffer(_dc);

    // PROFILING_INIT(t_blit);
    // PROFILING_START(t_blit);

#define CONV(_b0)    (((0b11111000000000000000000000&_b0)>>10) | ((0b000001111110000000000&_b0)>>5) | ((0b0000000000011111&_b0)))
#define EXPAND(_b0)  (((0b1111100000000000 & _b0) << 10) | ((0b0000011111100000 & _b0) << 5) | ((0b0000000000011111 & _b0)))

    int y_src = 0;
    int y_dst = 0;
    int w = currentUpdate->width;
    int h = currentUpdate->height;
    for (; y_src < h; y_src += 3, y_dst += 5) {
        int x_src = 0;
        int x_dst = 0;
        for (; x_src < w; x_src += 1, x_dst += 2) {
            uint16_t *src_col = &((uint16_t *)currentUpdate->buffer)[(y_src * w) + x_src];
            uint32_t b0 = EXPAND(src_col[w * 0]);
            uint32_t b1 = EXPAND(src_col[w * 1]);
            uint32_t b2 = EXPAND(src_col[w * 2]);

            dest[((y_dst + 0) * WIDTH) + x_dst] = CONV(b0) | 1;
            dest[((y_dst + 1) * WIDTH) + x_dst] = CONV((b0+b1)>>1) | 1;
            dest[((y_dst + 2) * WIDTH) + x_dst] = CONV(b1) | 1;
            dest[((y_dst + 3) * WIDTH) + x_dst] = CONV((b1+b2)>>1) | 1;
            dest[((y_dst + 4) * WIDTH) + x_dst] = CONV(b2) | 1;

            dest[((y_dst + 0) * WIDTH) + x_dst + 1] = CONV(b0) | 1;
            dest[((y_dst + 1) * WIDTH) + x_dst + 1] = CONV((b0+b1)>>1) | 1;
            dest[((y_dst + 2) * WIDTH) + x_dst + 1] = CONV(b1) | 1;
            dest[((y_dst + 3) * WIDTH) + x_dst + 1] = CONV((b1+b2)>>1) | 1;
            dest[((y_dst + 4) * WIDTH) + x_dst + 1] = CONV(b2) | 1;
        }
    }

    // PROFILING_END(t_blit);

#ifdef PROFILING_ENABLED
    // printf("Blit: %d us\n", (1000000 * PROFILING_DIFF(t_blit)) / t_blit_t0.SecondFraction);
#endif
    unlockVideo(_dc);
}


__attribute__((optimize("unroll-loops")))
static inline void screen_blit(void) {
    // static uint32_t lastFPSTime = 0;
    // static uint32_t frames = 0;
    // uint32_t currentTime = HAL_GetTick();
    // uint32_t delta = currentTime - lastFPSTime;

    // frames++;

    // if (delta >= 1000) {
    //     int fps = (10000 * frames) / delta;
    //     printf("FPS: %d.%d, frames %ld, delta %ld ms, skipped %d\n", fps / 10, fps % 10, delta, frames, common_emu_state.skipped_frames);
    //     frames = 0;
    //     common_emu_state.skipped_frames = 0;
    //     lastFPSTime = currentTime;
    // }

    int w1 = currentUpdate->width;
    int h1 = currentUpdate->height;
    int w2 = 266;
    int h2 = 240;

    int x_ratio = (int)((w1<<16)/w2) +1;
    int y_ratio = (int)((h1<<16)/h2) +1;
    int hpad = 27;
    int x2, y2 ;

    uint16_t* screen_buf = (uint16_t*)currentUpdate->buffer;
    display_context_t _dc = lockVideo(1);
    uint16_t *dest = (uint16_t *) __get_buffer(_dc);

    // PROFILING_INIT(t_blit);
    // PROFILING_START(t_blit);

    for (int i=0;i<h2;i++) {
        for (int j=0;j<w2;j++) {
            x2 = ((j*x_ratio)>>16) ;
            y2 = ((i*y_ratio)>>16) ;
            uint16_t b2 = screen_buf[(y2*w1)+x2];
            dest[(i*WIDTH)+j+hpad] = b2;
        }
    }

    // PROFILING_END(t_blit);

#ifdef PROFILING_ENABLED
    // printf("Blit: %d us\n", (1000000 * PROFILING_DIFF(t_blit)) / t_blit_t0.SecondFraction);
#endif

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
    // fb.blit_func = &screen_blit;
    fb.blit_func = &screen_blit_v3to5;

    emu_init();

    //pal_set_dmg(odroid_settings_Palette_get());
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
