// Copyright (c) 2022 Konrad Beckmann
// SPDX-License-Identifier: MIT

#pragma once

#include <stdint.h>
#include "mailbox.h"


typedef enum {
    COMMAND_INVALID = 0,
    COMMAND_PEEK    = 1,
    COMMAND_POKE    = 2,
    COMMAND_EXECUTE = 3,
} command_type_t;

typedef struct {
    uint32_t type;
    union {
        struct {
            uint32_t address;
            uint32_t length;
        } peek;

        struct {
            uint32_t address;
            uint32_t data[MAILBOX_PAYLOAD_WORDS - 2];
        } poke;

        struct {
            uint32_t address;
        } execute;
    };
} command_t;
