// Modified version of examples/spritemap/spritemap.c

#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <stdint.h>
#include <libdragon.h>
#include <cop1.h>

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

// disp: Display context
// t[8]: Profiling timestamps
typedef void (scene_t)(display_context_t disp, uint32_t t[8]);

// Unfortunately needed to access the raw frame buffer form libdragon
#define __get_buffer(x) __safe_buffer[(x) - 1]
extern void *__safe_buffer[3];

void scene0(display_context_t disp, uint32_t t[8]);
void scene1(display_context_t disp, uint32_t t[8]);
void scene2(display_context_t disp, uint32_t t[8]);


volatile uint32_t animcounter = 0;

static void printText(display_context_t dc, char *msg, int x, int y)
{
    if (dc)
        graphics_draw_text(dc, x*8, y*8, msg);
}

static void vblCallback(void)
{
    animcounter++;
}

scene_t *scenes[] = {
    &scene2,
    &scene1,
    &scene0,
};

int main(void)
{
    char temp[128];
    uint32_t prev_buttons = 0;
    uint32_t t_end1 = 0;
    uint32_t t_end2 = 0;
    uint32_t t_delta, t_delta_ms;
    uint32_t scene = 0;
    uint32_t cycle = 0;
    uint32_t print_stats = 1;

    /* enable interrupts (on the CPU) */
    init_interrupts();
    debug_init_isviewer();

    /* Initialize peripherals */
    display_init(RESOLUTION_320x240, DEPTH_16_BPP, 2, GAMMA_NONE, ANTIALIAS_OFF);
    set_VI_interrupt(1, 590);

    dfs_init(DFS_DEFAULT_LOCATION);
    rdp_init();
    controller_init();
    register_VI_handler(vblCallback);

    /* Main loop test */
    while(1)
    {
        display_context_t disp = 0;

        /* Grab a render buffer */
        while(!(disp = display_lock()));

        uint32_t t[8];
        static uint32_t t0_old;
        t[0] = TICKS_READ();
        uint32_t t_frame = t[0] - t0_old;
        t0_old = t[0];

        /* Set the text output color */
        graphics_set_color(0x0, 0xFFFFFFFF);

        // Draw active scene
        scenes[scene](disp, t);

        // Cycle through the scenes automatically
        if (cycle && (animcounter % 100 == 0)) {
            scene = (scene + 1) % ARRAY_SIZE(scenes);
        }

        if (print_stats) {
            t_delta_ms = t_frame / (TICKS_PER_SECOND / 1000);

            sprintf(temp, "t_frame=%ld (%ld ms)", t_frame, t_delta_ms);
            printText(disp, temp, 15, 5);

            t_delta = t[7] - t[0];
            t_delta_ms = t_delta / (TICKS_PER_SECOND / 1000);

            sprintf(temp, "t7-t0=%ld (%ld ms)", t_delta, t_delta_ms);
            printText(disp, temp, 15, 7);

            t_delta = t_end2 - t_end1;
            t_delta_ms = t_delta / (TICKS_PER_SECOND / 1000);

            sprintf(temp, "t_end=%ld (%ld ms)", t_delta, t_delta_ms);
            printText(disp, temp, 15, 9);

            for (int i = 0; i < 7; i++) {
                t_delta = t[i + 1] - t[i];
                t_delta_ms = t_delta / (TICKS_PER_SECOND / 1000);

                sprintf(temp, "t%d-t%d=%ld (%ld ms)", i + 1, i, t_delta, t_delta_ms);
                printText(disp, temp, 15, 11 + 2*i);
            }
        }

        /* Force backbuffer flip */
        display_show(disp);

        t_end1 = t[7];
        t_end2 = TICKS_READ();

        /* Do we need to switch video displays? */
        controller_scan();
        struct controller_data keys = get_keys_down();


        if (prev_buttons != keys.c[0].data) {
            if (keys.c[0].A) {
                scene = (scene + 1) % ARRAY_SIZE(scenes);
            }
            if (keys.c[0].B) {
                cycle = !cycle;
            }
            if (keys.c[0].Z) {
                print_stats = !print_stats;
            }
        }

        prev_buttons = keys.c[0].data;
    }
}
