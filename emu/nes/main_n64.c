// Based on
// https://github.com/DragonMinded/libdragon/blob/trunk/examples/vtest/vtest.c

#include <libdragon.h>
#include <malloc.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <odroid_system.h>

#include "crc32.h"
#include "porting.h"

// #include <bitmap.h>
#include <nes.h>
#include <nes_input.h>
#include <nes_state.h>
#include <nofrendo.h>
#include <osd.h>
#include <string.h>

#define APP_ID 30

#define AUDIO_SAMPLE_RATE (48000)
#define AUDIO_BUFFER_LENGTH (AUDIO_SAMPLE_RATE / 60)

volatile int gTicks; /* incremented every vblank */
static uint romCRC32;
static odroid_gamepad_state_t joystick1;

extern unsigned int cart_rom_len;
extern const unsigned char cart_rom[];

int WIDTH = 320;
int HEIGHT = 240;

void osd_shutdown()
{
   //
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

/* text functions */
void drawText(display_context_t dc, char *msg, int x, int y)
{
  if (dc)
    graphics_draw_text(dc, x, y, msg);
}

void printText(display_context_t dc, char *msg, int x, int y)
{
  if (dc)
    graphics_draw_text(dc, x * 8, y * 8, msg);
}

/* vblank callback */
void vblCallback(void) {
    gTicks++;
}

void delay(int cnt)
{
  int then = gTicks + cnt;
  while (then > gTicks)
    ;
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

void odroid_display_force_refresh(void)
{
    // forceVideoRefresh = true;
}

int osd_init()
{
   return 0;
}


void osd_vsync()
{
}


static rgb_t *palette = NULL;
static uint16_t palette565[256];
static uint32_t palette_spaced_565[256];


void osd_setpalette(rgb_t *pal)
{
    palette = pal;
    for (int i = 0; i < 64; i++)
    {
        // uint16_t c = (pal[i].b>>3) | ((pal[i].g>>2)<<5) | ((pal[i].r>>3)<<11);
        uint16_t c = ((pal[i].b>>2) | 1) | ((pal[i].g>>3)<<6) | ((pal[i].r>>3)<<11);

        // The upper bits are used to indicate background and transparency.
        // They need to be indexed as well.
        palette565[i]        = c;
        palette565[i | 0x40] = c;
        palette565[i | 0x80] = c;

        uint32_t sc = ((0b1111100000000000&c)<<10) | ((0b0000011111100000&c)<<5) | ((0b0000000000011111&c));
        palette_spaced_565[i] = sc;
        palette_spaced_565[i | 0x40] = sc;
        palette_spaced_565[i | 0x80] = sc;
    }
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


#define blit blit_normal
// #define blit blit_4to5

__attribute__((optimize("unroll-loops")))
static inline void blit_normal(bitmap_t *bmp, uint16_t *framebuffer) {
    const int w1 = bmp->width;
    const int w2 = 320;
    const int h2 = 240;
    const int hpad = 27;

    for (int y = 0; y < h2; y++) {
        uint8_t  *src_row  = bmp->line[y];
        uint16_t *dest_row = &framebuffer[y * w2 + hpad];
        for (int x = 0; x < w1; x++) {
            dest_row[x] = palette565[src_row[x]];
        }
    }
}

#define CONV(_b0) ((0b11111000000000000000000000&_b0)>>10) | ((0b000001111110000000000&_b0)>>5) | ((0b0000000000011111&_b0));

__attribute__((optimize("unroll-loops")))
static void blit_4to5(bitmap_t *bmp, uint16_t *framebuffer) {
    int w1 = bmp->width;
    int w2 = WIDTH;
    int h2 = 240;

    // 1767 us

    for (int y = 0; y < h2; y++) {
        uint8_t  *src_row  = bmp->line[y];
        uint16_t *dest_row = &framebuffer[y * w2];
        for (int x_src = 0, x_dst=0; x_src < w1; x_src+=4, x_dst+=5) {
            uint32_t b0 = palette_spaced_565[src_row[x_src]];
            uint32_t b1 = palette_spaced_565[src_row[x_src+1]];
            uint32_t b2 = palette_spaced_565[src_row[x_src+2]];
            uint32_t b3 = palette_spaced_565[src_row[x_src+3]];

            dest_row[x_dst]   = CONV(b0);
            dest_row[x_dst+1] = CONV((b0+b0+b0+b1)>>2);
            dest_row[x_dst+2] = CONV((b1+b2)>>1);
            dest_row[x_dst+3] = CONV((b2+b2+b2+b3)>>2);
            dest_row[x_dst+4] = CONV(b3);
        }
    }
}

void osd_blitscreen(bitmap_t *bmp)
{
    // static uint32_t lastFPSTime = 0;
    // static uint32_t frames = 0;
    // uint32_t currentTime = HAL_GetTick();
    // uint32_t delta = currentTime - lastFPSTime;

    // frames++;

    // if (delta >= 1000) {
    //     int fps = (10000 * frames) / delta;
    //     printf("FPS: %d.%d, frames %ld, delta %ld ms, skipped %d\n", fps / 10, fps % 10, frames, delta, common_emu_state.skipped_frames);
    //     frames = 0;
    //     common_emu_state.skipped_frames = 0;
    //     vsync_wait_ms = 0;
    //     lastFPSTime = currentTime;
    // }

    // PROFILING_INIT(t_blit);
    // PROFILING_START(t_blit);

    display_context_t _dc = lockVideo(1);
    uint16_t *fb = (uint16_t *) __get_buffer(_dc);

    blit(bmp, fb);
    // common_ingame_overlay();
    unlockVideo(_dc);

    // PROFILING_END(t_blit);

#ifdef PROFILING_ENABLED
    // printf("Blit: %d us\n", (1000000 * PROFILING_DIFF(t_blit)) / t_blit_t0.SecondFraction);
#endif
}


void osd_getinput(void)
{
    uint8 pad0 = 0;

    odroid_input_read_gamepad(&joystick1);

    if (joystick1.values[ODROID_INPUT_START])  pad0 |= INP_PAD_START;
    if (joystick1.values[ODROID_INPUT_SELECT]) pad0 |= INP_PAD_SELECT;
    if (joystick1.values[ODROID_INPUT_UP])     pad0 |= INP_PAD_UP;
    if (joystick1.values[ODROID_INPUT_RIGHT])  pad0 |= INP_PAD_RIGHT;
    if (joystick1.values[ODROID_INPUT_DOWN])   pad0 |= INP_PAD_DOWN;
    if (joystick1.values[ODROID_INPUT_LEFT])   pad0 |= INP_PAD_LEFT;
    if (joystick1.values[ODROID_INPUT_A])      pad0 |= INP_PAD_A;
    if (joystick1.values[ODROID_INPUT_B])      pad0 |= INP_PAD_B;

    input_update(INP_JOYPAD0, pad0);
}


size_t osd_getromdata(unsigned char **data)
{
  *data = (unsigned char *)cart_rom;
  return cart_rom_len;
}

uint osd_getromcrc() { return romCRC32; }

void osd_loadstate() {}

static bool SaveState(char *pathName) { return true; }

static bool LoadState(char *pathName) { return true; }

/* main code entry point */
int main(void)
{
  init_n64();

  odroid_system_init(APP_ID, AUDIO_SAMPLE_RATE);
  odroid_system_emu_init(&LoadState, &SaveState, NULL);

  printf("app_main ROM: cart_rom_len=%d\n", cart_rom_len);

  romCRC32 = crc32_le(0, (const uint8_t *)(cart_rom + 16), cart_rom_len - 16);

  int region = NES_NTSC;
  // region = NES_PAL;

  printf("Nofrendo start!\n");

  // nofrendo_start("Rom name (E).nes", NES_PAL, AUDIO_SAMPLE_RATE);
  nofrendo_start("Rom name (USA).nes", region, AUDIO_SAMPLE_RATE, false);

  return 0;
}
