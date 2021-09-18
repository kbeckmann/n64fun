#include "common.h"

// 4x supersampling test. Super slow.

void scene5(display_context_t disp, uint32_t t[8])
{
    if (__width != 512 || __height != 240 || __bitdepth != 2) {
        // unsupported resolution
        return;
    }

    static int initialized;
    static uint8_t *scene_data_start;
    static uint8_t *scene_data;

    if (!initialized) {
        initialized = 1;
        scene_data_start = niccc_get_data();
        scene_data = scene_data_start;
    }

    struct controller_data keys = get_keys_pressed();
    if (keys.c[0].start) {
        // Reset when pressing start
        scene_data = scene_data_start;
    }


    t[1] = TICKS_READ();

    #define buf_w (512*2)
    #define buf_h (240*2)
    static uint16_t off_buf[buf_w * buf_h] __attribute__((aligned(64)));
    rdp_sync(SYNC_PIPE);
    rdp_set_clipping( 0, 0, buf_w-1, buf_h-1 );
    rdp_attach_buffer(off_buf, buf_w);
    render_niccc(&scene_data, buf_w-1, buf_h);

    t[2] = TICKS_READ();

    uint16_t *fb = __get_buffer(disp);

    // Perform 1:2 bilinear downscaling (slow)
    for (int y = 0; y < __height; y++) {
        // Source
        uint16_t *row0_s = &off_buf[buf_w * (2*y)];
        uint16_t *row1_s = &off_buf[buf_w * (2*y + 1)];

        // Destination
        uint16_t *row_d  = &fb[__width * y];

        for (int x = 0; x < __width; x++) {
            uint16_t samp00 = row0_s[2*x];
            uint16_t samp01 = row0_s[2*x+1];
            uint16_t samp10 = row1_s[2*x];
            uint16_t samp11 = row1_s[2*x+1];

            #define GET_R(_x) (((_x) >> (1 + 5 + 5)) & 0x1F)
            #define GET_G(_x) (((_x) >> (1 + 5    )) & 0x1F)
            #define GET_B(_x) (((_x) >> (1        )) & 0x1F)

            #define SET_R(_x) (((_x) & 0x1F) << (1 + 5 + 5))
            #define SET_G(_x) (((_x) & 0x1F) << (1 + 5    ))
            #define SET_B(_x) (((_x) & 0x1F) << (1        ))

            uint16_t col = (
                SET_R((GET_R(samp00) + GET_R(samp01) + GET_R(samp10) + GET_R(samp11)) / 4) |
                SET_G((GET_G(samp00) + GET_G(samp01) + GET_G(samp10) + GET_G(samp11)) / 4) |
                SET_B((GET_B(samp00) + GET_B(samp01) + GET_B(samp10) + GET_B(samp11)) / 4) |
                1
            );

            row_d[x] = col;
            // row_d[x] = samp00;
            // row_d[x] = SET_R(x) | SET_G(y) | 1;
        }
    }

    t[3] = TICKS_READ();
    t[4] = t[5] = t[6] = t[7] = TICKS_READ();
}
