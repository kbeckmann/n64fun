#include <libdragon.h>
#include "odroid_system.h"
#include "odroid_input.h"

unsigned short gButtons = 0;
struct controller_data gKeys;

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

void odroid_input_read_gamepad(odroid_gamepad_state_t* out_state)
{
    gButtons = getButtons(0);

    out_state->values[ODROID_INPUT_A]      = !! A_BUTTON(gButtons);
    out_state->values[ODROID_INPUT_B]      = !! B_BUTTON(gButtons);
    out_state->values[ODROID_INPUT_START]  = !! START_BUTTON(gButtons);
    out_state->values[ODROID_INPUT_SELECT] = !! Z_BUTTON(gButtons);
    out_state->values[ODROID_INPUT_UP]     = !! DU_BUTTON(gButtons);
    out_state->values[ODROID_INPUT_DOWN]   = !! DD_BUTTON(gButtons);
    out_state->values[ODROID_INPUT_LEFT]   = !! DL_BUTTON(gButtons);
    out_state->values[ODROID_INPUT_RIGHT]  = !! DR_BUTTON(gButtons);
}

void odroid_input_wait_for_key(odroid_gamepad_key_t key, bool pressed)
{
}

