// Copyright (c) 2022 Konrad Beckmann
// SPDX-License-Identifier: MIT

#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <stdint.h>
#include <libdragon.h>

#include "mailbox.h"
#include "command.h"


uint32_t payload_rx[MAILBOX_PAYLOAD_WORDS] __attribute__((aligned (4)));
uint32_t payload_tx[MAILBOX_PAYLOAD_WORDS] __attribute__((aligned (4)));

// should end up at 0x80100000
struct __attribute__((aligned (1024*1024)))
{
    uint32_t program[1024];
    uint32_t data[1024];
    struct {
        uint64_t cpu[32];
        uint64_t fpu[32];
        uint64_t lo;
        uint64_t hi;
        uint32_t fcr31;
    } regs;
} scratch;

const uint64_t *regs_address = &scratch.regs;

void call_and_save(uint32_t address);

__attribute__ ((noinline)) void call(uint32_t address)
{
    data_cache_writeback_invalidate_all();
    inst_cache_invalidate_all();
    MEMORY_BARRIER();

    call_and_save(address);

}

int main(void)
{
    command_t *cmd_rx = (command_t *) payload_rx;

    mailbox_init();

    memset(&scratch, 'A', sizeof(scratch));

    while (1) {
        int32_t len_rx = mailbox_rx(payload_rx, true, true, 100000);

        if (len_rx < 0) {
            continue;
        }

        switch (cmd_rx->type) {
            case COMMAND_PEEK:
                assert(cmd_rx->peek.length <= MAILBOX_PAYLOAD_WORDS);
                memcpy(payload_tx, cmd_rx->peek.address, cmd_rx->peek.length * 4);
                mailbox_tx(payload_tx, cmd_rx->peek.length, true, true, 100000);
                break;

            case COMMAND_POKE:
                memcpy(cmd_rx->peek.address, cmd_rx->poke.data, len_rx * 4);
                break;

            case COMMAND_EXECUTE:
                call(cmd_rx->execute.address);
                break;
        }
    }

    return 0;
}
