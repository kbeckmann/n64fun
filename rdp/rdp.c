// Modified version of examples/spritemap/spritemap.c

#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <stdint.h>
#include <libdragon.h>

static volatile uint32_t animcounter = 0;

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


void printText(display_context_t dc, char *msg, int x, int y)
{
    if (dc)
        graphics_draw_text(dc, x*8, y*8, msg);
}

void vblCallback(void)
{
    animcounter++;
}

int main(void)
{
    char temp[128];
    int mode = 1;

    /* enable interrupts (on the CPU) */
    init_interrupts();

    /* Initialize peripherals */
    display_init( RESOLUTION_320x240, DEPTH_16_BPP, 2, GAMMA_NONE, ANTIALIAS_OFF );
    set_VI_interrupt(1, 590);

    dfs_init( DFS_DEFAULT_LOCATION );
    rdp_init();
    controller_init();
    register_VI_handler(vblCallback);

    /* Read in single sprite */
    int fp = dfs_open("/mudkip.sprite");
    sprite_t *mudkip = malloc( dfs_size( fp ) );
    dfs_read( mudkip, 1, dfs_size( fp ), fp );
    dfs_close( fp );
    
    fp = dfs_open("/earthbound.sprite");
    sprite_t *earthbound = malloc( dfs_size( fp ) );
    dfs_read( earthbound, 1, dfs_size( fp ), fp );
    dfs_close( fp );

    fp = dfs_open("/plane.sprite");
    sprite_t *plane = malloc( dfs_size( fp ) );
    dfs_read( plane, 1, dfs_size( fp ), fp );
    dfs_close( fp );

    /* Main loop test */
    while(1) 
    {
        static display_context_t disp = 0;

        /* Grab a render buffer */
        while( !(disp = display_lock()) );

        uint32_t t[8];
        static uint32_t t0_old;
        t[0] = TICKS_READ();
        uint32_t t_frame = t[0] - t0_old;
        t0_old = t[0];

        /* Set the text output color */
        graphics_set_color( 0x0, 0xFFFFFFFF );


        switch( mode )
        {
            case 0:

                /*Fill the screen */
                // graphics_fill_screen( disp, 0xFFFFFFFF );
                memset(__get_buffer(disp), '\xFF', 320*240*2);

                t[1] = TICKS_READ();

                /* Software spritemap test */
                graphics_draw_text( disp, 20, 20, "Software spritemap test" );

                t[2] = TICKS_READ();

                /* Display a stationary sprite of adequate size to fit in TMEM */
                graphics_draw_sprite_trans( disp, 20, 50, plane );

                t[3] = TICKS_READ();

                /* Display a stationary sprite to demonstrate backwards compatibility */
                graphics_draw_sprite_trans( disp, 50, 50, mudkip );

                t[4] = TICKS_READ();

                /* Display walking NESS animation */
                graphics_draw_sprite_stride( disp, 20, 100, earthbound, ((animcounter / 15) & 1) ? 1: 0 );

                t[5] = TICKS_READ();

                /* Display rotating NESS animation */
                graphics_draw_sprite_stride( disp, 50, 100, earthbound, ((animcounter / 8) & 0x7) * 2 );

                t[6] = t[7] = TICKS_READ();

                break;
            case 1:
            {
                /* Assure RDP is ready for new commands */
                rdp_sync( SYNC_PIPE );
                t[1] = TICKS_READ();

                /* Remove any clipping windows */
                rdp_set_default_clipping();

                /* Attach RDP to display */
                rdp_attach_display( disp );

                /* Ensure the RDP is ready to receive sprites */
                rdp_sync( SYNC_PIPE );
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
                rdp_load_texture( 0, 0, MIRROR_DISABLED, plane );
                t[4] = TICKS_READ();

                /* Display a stationary sprite of adequate size to fit in TMEM */
                rdp_draw_sprite( 0, 20, 50, MIRROR_DISABLED );
                t[5] = TICKS_READ();

                /* Since the RDP is very very limited in texture memory, we will use the spritemap feature to display
                   all four pieces of this sprite individually in order to use the RDP at all */
                for( int i = 0; i < 4; i++ )
                {
                    /* Ensure the RDP is ready to receive sprites */
                    rdp_sync( SYNC_PIPE );

                    /* Load the sprite into texture slot 0, at the beginning of memory, without mirroring */
                    rdp_load_texture_stride( 0, 0, MIRROR_DISABLED, mudkip, i );
                
                    /* Display a stationary sprite to demonstrate backwards compatibility */
                    rdp_draw_sprite( 0, 50 + (20 * (i % 2)), 50 + (20 * (i / 2)), MIRROR_DISABLED );
                }

                /* Ensure the RDP is ready to receive sprites */
                rdp_sync( SYNC_PIPE );

                /* Load the sprite into texture slot 0, at the beginning of memory, without mirroring */
                rdp_load_texture_stride( 0, 0, MIRROR_DISABLED, earthbound, ((animcounter / 15) & 1) ? 1: 0 );
                
                /* Display walking NESS animation */
                rdp_draw_sprite( 0, 20, 100, MIRROR_DISABLED );

                /* Ensure the RDP is ready to receive sprites */
                rdp_sync( SYNC_PIPE );

                /* Load the sprite into texture slot 0, at the beginning of memory, without mirroring */
                rdp_load_texture_stride( 0, 0, MIRROR_DISABLED, earthbound, ((animcounter / 8) & 0x7) * 2 );
                
                /* Display rotating NESS animation */
                rdp_draw_sprite( 0, 50, 100, MIRROR_DISABLED );

                t[6] = TICKS_READ();
                /* Inform the RDP we are finished drawing and that any pending operations should be flushed */
                // Actually does a FULL_SYNC which can be quite heavy.
                rdp_detach_display();
                t[7] = TICKS_READ();

                /* Hardware spritemap test */
                graphics_draw_text( disp, 20, 20, "Hardware spritemap test" );

                break;
            }
        }

        uint32_t t_delta, t_delta_ms;


        t_delta_ms = t_frame / (TICKS_PER_SECOND / 1000);

        sprintf(temp, "t_frame=%d (%d ms)", t_frame, t_delta_ms);
        printText(disp, temp, 15, 5);

        t_delta = t[7] - t[0];
        t_delta_ms = t_delta / (TICKS_PER_SECOND / 1000);

        sprintf(temp, "t7-t0=%d (%d ms)", t_delta, t_delta_ms);
        printText(disp, temp, 15, 7);

        for (int i = 0; i < 7; i++) {
            t_delta = t[i + 1] - t[i];
            t_delta_ms = t_delta / (TICKS_PER_SECOND / 1000);

            sprintf(temp, "t%d-t%d=%d (%d ms)", i + 1, i, t_delta, t_delta_ms);
            printText(disp, temp, 15, 10 + 2*i);
        }

        /* Force backbuffer flip */
        display_show(disp);

        /* Do we need to switch video displays? */
        controller_scan();
        struct controller_data keys = get_keys_down();

        if( keys.c[0].A )
        {
            /* Lazy switching */
            mode = 1 - mode;
        }
    }
}
