// Copyright (c) 2022 Konrad Beckmann
// SPDX-License-Identifier: MIT

#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <stdint.h>
#include <libdragon.h>


int main(void)
{
    // enable debug for emulators
    debug_init(DEBUG_FEATURE_LOG_ISVIEWER);

    int i = 0;
    while (1) {
        i++;
        // io_write()
    }

    return 0;
}
