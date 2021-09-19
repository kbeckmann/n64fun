// Copyright (c) 2021 Konrad Beckmann
// SPDX-License-Identifier: MIT

#include "common.h"

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

    rdp_enable_primitive_fill();
    rdp_set_primitive_color(0xFFFFFFFF);
    rdp_draw_filled_rectangle(0, 0, 320, 240);

    t[2] = TICKS_READ();

    float theta = ((float) g_frame) / 100;

    /* Right-angled triangle
    *
    * 1     2
    * _____ 
    * |   /
    * |  /
    * | /
    * |/
    *
    * 3
    *
    */

    // Triangle coords
    float x1 = 0;
    float y1 = 0;
    float x2 = 1.0f;
    float y2 = 0;
    float x3 = 0.0f;
    float y3 = 1.0f;

    // Sin/Cos for rotation
    // theta = acosf(0) * 3.5;
    float s = sin(theta);
    float c = cos(theta);

    // Translate
    float tx = 100.0f;
    float ty = 100.0f;

    // Scale
    float sx = 100.0f;
    float sy = 100.0f;
    t[3] = TICKS_READ();

    // Scale and translate
    rdp_set_primitive_color(0x0);

    // TODO: Set Other Modes to enable alpha blending

    rdp_enable_blend_fill();
    rdp_set_blend_color(0x0000FF7F);
    rdp_draw_filled_triangle(x1 * sx + tx, y1 * sy + ty,
                             x2 * sx + tx, y2 * sy + ty,
                             x3 * sx + tx, y3 * sy + ty);
    t[4] = TICKS_READ();

    // Rotate, scale then translate
    rdp_enable_blend_fill();
    rdp_set_blend_color(0xFF00007F);
    rdp_draw_filled_triangle(( x1 * c + y1 * s) * sx + tx,
                             (-x1 * s + y1 * c) * sy + ty,
                             ( x2 * c + y2 * s) * sx + tx,
                             (-x2 * s + y2 * c) * sy + ty,
                             ( x3 * c + y3 * s) * sx + tx,
                             (-x3 * s + y3 * c) * sy + ty);
    t[5] = TICKS_READ();

    // Center point
    rdp_set_primitive_color(0x00000000);
    rdp_draw_filled_rectangle(tx-1, ty-1, tx+1, ty+1);

    t[6] = TICKS_READ();

    /* Inform the RDP we are finished drawing and that any pending operations should be flushed */
    // Actually does a FULL_SYNC which can be quite heavy.
    rdp_detach_display();
    t[7] = TICKS_READ();
}
