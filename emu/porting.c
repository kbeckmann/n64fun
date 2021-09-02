#include "porting.h"
// #include <SDL2/SDL.h>

uint32_t HAL_GetTick(void)
{
    // return SDL_GetTicks();
    static int i;
    return i++;
}

void wdog_refresh(void)
{

}
