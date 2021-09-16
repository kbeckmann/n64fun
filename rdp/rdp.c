// Modified version of examples/spritemap/spritemap.c

#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <stdint.h>
#include <libdragon.h>

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

// disp: Display context
// t[8]: Profiling timestamps
typedef void (scene_t)(display_context_t disp, uint32_t t[8]);

// Unfortunately needed to access the raw frame buffer form libdragon
#define __get_buffer(x) __safe_buffer[(x) - 1]
extern void *__safe_buffer[3];

static volatile uint32_t animcounter = 0;

static void printText(display_context_t dc, char *msg, int x, int y)
{
    if (dc)
        graphics_draw_text(dc, x*8, y*8, msg);
}

static void vblCallback(void)
{
    animcounter++;
}


static void scene0(display_context_t disp, uint32_t t[8])
{
    // Just clears the screen, nothing else

    rdp_sync(SYNC_PIPE);
    t[1] = TICKS_READ();

    rdp_set_default_clipping();
    rdp_attach_display(disp);
    rdp_sync(SYNC_PIPE);
    t[2] = TICKS_READ();

    rdp_enable_primitive_fill();
    rdp_set_primitive_color(0xFFFFFFFF);
    rdp_draw_filled_rectangle(0, 0, 320, 240);
    t[3] = TICKS_READ();
    t[4] = TICKS_READ();
    t[5] = TICKS_READ();
    t[6] = TICKS_READ();

    rdp_detach_display();
    t[7] = TICKS_READ();
}

static void scene1(display_context_t disp, uint32_t t[8])
{
    static int initialized;
    static sprite_t *mudkip;
    static sprite_t *earthbound;
    static sprite_t *plane;

    if (!initialized) {
        initialized = 1;

        /* Read in single sprite */
        int fp = dfs_open("/mudkip.sprite");
        mudkip = malloc(dfs_size(fp));
        dfs_read(mudkip, 1, dfs_size(fp), fp);
        dfs_close(fp);

        fp = dfs_open("/earthbound.sprite");
        earthbound = malloc(dfs_size(fp));
        dfs_read(earthbound, 1, dfs_size(fp), fp);
        dfs_close(fp);

        fp = dfs_open("/plane.sprite");
        plane = malloc(dfs_size(fp));
        dfs_read(plane, 1, dfs_size(fp), fp);
        dfs_close(fp);
    }

    /* Assure RDP is ready for new commands */
    rdp_sync(SYNC_PIPE);
    t[1] = TICKS_READ();

    /* Remove any clipping windows */
    rdp_set_default_clipping();

    /* Attach RDP to display */
    rdp_attach_display(disp);

    /* Ensure the RDP is ready to receive sprites */
    rdp_sync(SYNC_PIPE);
    t[2] = TICKS_READ();

    rdp_enable_primitive_fill();
    rdp_set_primitive_color(0xFFFFFFFF);
    rdp_draw_filled_rectangle(0, 0, 320, 240);

    rdp_set_primitive_color(graphics_make_color(0xFF, 0x00, 0xFF, 0xFF));
    rdp_draw_filled_rectangle(animcounter % (320-50), 0, (animcounter % (320-50)) + 50, 240);

    t[3] = TICKS_READ();

    /* Enable sprite display instead of solid color fill */
    rdp_enable_texture_copy();

    /* Load the sprite into texture slot 0, at the beginning of memory, without mirroring */
    rdp_load_texture(0, 0, MIRROR_DISABLED, plane);
    t[4] = TICKS_READ();

    /* Display a stationary sprite of adequate size to fit in TMEM */
    rdp_draw_sprite(0, 20, 50, MIRROR_DISABLED);
    t[5] = TICKS_READ();

    /* Since the RDP is very very limited in texture memory, we will use the spritemap feature to display
        all four pieces of this sprite individually in order to use the RDP at all */
    for(int i = 0; i < 4; i++)
    {
        /* Ensure the RDP is ready to receive sprites */
        rdp_sync(SYNC_PIPE);

        /* Load the sprite into texture slot 0, at the beginning of memory, without mirroring */
        rdp_load_texture_stride(0, 0, MIRROR_DISABLED, mudkip, i);

        /* Display a stationary sprite to demonstrate backwards compatibility */
        rdp_draw_sprite(0, 50 + (20 * (i % 2)), 50 + (20 * (i / 2)), MIRROR_DISABLED);
    }

    /* Ensure the RDP is ready to receive sprites */
    rdp_sync(SYNC_PIPE);

    /* Load the sprite into texture slot 0, at the beginning of memory, without mirroring */
    rdp_load_texture_stride(0, 0, MIRROR_DISABLED, earthbound, ((animcounter / 15) & 1) ? 1: 0);

    /* Display walking NESS animation */
    rdp_draw_sprite(0, 20, 100, MIRROR_DISABLED);

    /* Ensure the RDP is ready to receive sprites */
    rdp_sync(SYNC_PIPE);

    /* Load the sprite into texture slot 0, at the beginning of memory, without mirroring */
    rdp_load_texture_stride(0, 0, MIRROR_DISABLED, earthbound, ((animcounter / 8) & 0x7) * 2);

    /* Display rotating NESS animation */
    rdp_draw_sprite(0, 50, 100, MIRROR_DISABLED);

    t[6] = TICKS_READ();
    /* Inform the RDP we are finished drawing and that any pending operations should be flushed */
    // Actually does a FULL_SYNC which can be quite heavy.
    rdp_detach_display();
    t[7] = TICKS_READ();
}

static void scene2(display_context_t disp, uint32_t t[8])
{
    /* Assure RDP is ready for new commands */
    rdp_sync(SYNC_PIPE);
    t[1] = TICKS_READ();

    /* Remove any clipping windows */
    rdp_set_default_clipping();

    /* Attach RDP to display */
    rdp_attach_display(disp);

    /* Ensure the RDP is ready to receive sprites */
    rdp_sync(SYNC_PIPE);
    t[2] = TICKS_READ();

    rdp_enable_primitive_fill();
    rdp_set_primitive_color(0xFFFFFFFF);
    rdp_draw_filled_rectangle(0, 0, 320, 240);

    rdp_set_primitive_color(graphics_make_color(0xFF, 0x00, 0xFF, 0xFF));
    
    t[3] = TICKS_READ();

    rdp_draw_filled_triangle(100, 100,
                             200, 100,
                             150, 200);


    t[4] = TICKS_READ();
    t[5] = TICKS_READ();
    t[6] = TICKS_READ();

    /* Inform the RDP we are finished drawing and that any pending operations should be flushed */
    // Actually does a FULL_SYNC which can be quite heavy.
    rdp_detach_display();
    t[7] = TICKS_READ();
}


scene_t *scenes[] = {
    &scene0,
    &scene1,
    &scene2,
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
