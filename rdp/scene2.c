#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <stdint.h>
#include <libdragon.h>

void scene2(display_context_t disp, uint32_t t[8])
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
