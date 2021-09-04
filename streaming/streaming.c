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
    /* enable interrupts (on the CPU) */
    init_interrupts();

    /* Initialize peripherals */
    display_init( RESOLUTION_320x240, DEPTH_32_BPP, 2, GAMMA_NONE, ANTIALIAS_OFF );
    set_VI_interrupt(1, 590);

    dfs_init( DFS_DEFAULT_LOCATION );

    register_VI_handler(vblCallback);

    controller_init();
}


static void mem_dump(display_context_t _dc, uint32_t *address, uint32_t len, int x, int y)
{
    char temp[128];
    uint32_t written = 0;

    union {
        uint32_t word;
        uint8_t  byte[4];
    } value;


    // xxd -p -c 16 < file.hex > file.bin
    for (int i = 0; i < (len + 15) / 16; i++) {
        for (int j = 0; j < 4; j++) {
            value.word = address[i * 4 + j];
            sprintf(temp, "%02x%02x%02x%02x",
                value.byte[0],
                value.byte[1],
                value.byte[2],
                value.byte[3]
            );
            printText(_dc, temp, x + j * 9, y + i);
            written += 4;
            if (written >= len) {
                break;
            }
        }
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

/**
 * @brief Align a memory address to 64 byte offset
 * 
 * @param[in] x
 *            Unaligned memory address
 *
 * @return An aligned address guaranteed to be >= the unaligned address
 */
#define ALIGN_64BYTE(x)     ((void *)(((uint32_t)(x)+63) & 0xFFFFFFC0))


/* main code entry point */
int main(void)
{
    display_context_t _dc;
    char temp[128];
    int res = 0;
    unsigned short buttons, previous = 0;

    uint32_t *buf_alloc = malloc(320*240*4 + 63);
    uint32_t *buf = ALIGN_64BYTE( buf_alloc );


    init_n64();

    uint32_t *asset = dfs_rom_addr("/frame.rgb");

    int file_idx = 0;
    int last_pos = 0;
    int frames = 0;
    while (1) {
        frames++;

        int width[6]  = { 320, 640, 256, 512, 512, 640 };
        // int height[6] = { 240, 480, 240, 480, 240, 240 };
        unsigned int color;

        _dc = lockVideo(1);

        #define CACHED_ADDR(x)    ((void *)(((uint32_t)(x)) & (~0xA0000000)))

        // uint32_t *dest = (uint32_t *) CACHED_ADDR(__get_buffer(_dc));
        uint32_t *dest = (uint32_t *) __get_buffer(_dc);

        dma_read(dest, asset, 320 * 240 * 4);

#if 0

        color = graphics_make_color(0x7F, 0x7F, 0x7F, 0xFF);
        graphics_fill_screen(_dc, color);

        color = graphics_make_color(0xFF, 0xFF, 0xFF, 0xFF);

        sprintf(temp, "p= %08X", (uint32_t) asset);
        printText(_dc, temp, 5, 3);

        dma_read(buf, asset, 256);
        mem_dump(_dc, asset, 256, 3, 13);

#endif

        unlockVideo(_dc);

        buttons = getButtons(0);

        if (A_BUTTON(buttons ^ previous)) {
            // A changed
            if (!A_BUTTON(buttons)) {
                resolution_t mode[6] = {
                    RESOLUTION_320x240,
                    RESOLUTION_640x480,
                    RESOLUTION_256x240,
                    RESOLUTION_512x480,
                    RESOLUTION_512x240,
                    RESOLUTION_640x240,
                };
                res++;
                res %= 6;
                display_close();
                display_init(mode[res], DEPTH_16_BPP, 2, GAMMA_NONE, ANTIALIAS_RESAMPLE);
                set_VI_interrupt(1, 590);
            }
        }

        previous = buttons;
    }

    return 0;
}
