#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <stdint.h>
#include <libdragon.h>

extern volatile uint32_t animcounter;

void scene1(display_context_t disp, uint32_t t[8])
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

        fprintf(stderr, "Initialized!\n");
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
