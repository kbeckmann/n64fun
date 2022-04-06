#include "mailbox.h"

#include <string.h>
#include <assert.h>
#include <libdragon.h>

#define MAILBOX_BASE (0x14000000 | 0xA0000000)
#define MAILBOX ((mailbox_t *) (MAILBOX_BASE))

static void read_words(uint32_t *dest, uint32_t *src, size_t words)
{
    while (words--) {
        *dest++ = io_read(src++);
    }
}

static void write_words(uint32_t *dest, uint32_t *src, size_t words)
{
    while (words--) {
        io_write(dest++, *src++);
    }
}

void mailbox_init(void)
{
    io_write(&MAILBOX->tx_state, MAILBOX_STATE_IDLE);
    io_write(&MAILBOX->tx_state_recv, MAILBOX_STATE_IDLE);
}

int32_t mailbox_rx(uint32_t *payload, bool block_ready, bool block_acked, uint32_t timeout)
{
    mailbox_state_t state;
    uint32_t counter = timeout;

    // Ensure we start with the IDLE state.
    io_write(&MAILBOX->tx_state_recv, MAILBOX_STATE_IDLE);

    if (block_ready) {
        do {
            state = io_read(&MAILBOX->rx_state);
            if (timeout != UINT32_MAX && counter-- == 0) {
                return MAILBOX_ERROR_IDLE_TIMEOUT;
            }
        } while (state != MAILBOX_STATE_DONE);
    } else {
        state = io_read(&MAILBOX->rx_state);
        if (state != MAILBOX_STATE_DONE) {
            return MAILBOX_ERROR_AGAIN;
        }
    }

    io_write(&MAILBOX->tx_state_recv, MAILBOX_STATE_BUSY);

    uint32_t length = io_read(&MAILBOX->rx_length);
    assert(length <= MAILBOX_PAYLOAD_WORDS);

    read_words(payload, MAILBOX->rx_payload, length);
    io_write(&MAILBOX->tx_state_recv, MAILBOX_STATE_DONE);

    counter = timeout;
    if (block_acked) {
        do {
            state = io_read(&MAILBOX->rx_state);
            if (timeout != UINT32_MAX && counter-- == 0) {
                return MAILBOX_ERROR_ACK_TIMEOUT;
            }
        } while (state != MAILBOX_STATE_IDLE);
    } else {
        return length;
    }

    io_write(&MAILBOX->tx_state_recv, MAILBOX_STATE_IDLE);

    return length;
}

int32_t mailbox_tx(uint32_t *payload, uint32_t length, bool block_ready, bool block_acked, uint32_t timeout)
{
    mailbox_state_t state;
    uint32_t counter = timeout;

    assert(length <= MAILBOX_PAYLOAD_WORDS);

    // Ensure we start with the IDLE state.
    io_write(&MAILBOX->tx_state, MAILBOX_STATE_IDLE);

    if (block_ready) {
        do {
            state = io_read(&MAILBOX->rx_state_recv);
            if (timeout != UINT32_MAX && counter-- == 0) {
                return MAILBOX_ERROR_IDLE_TIMEOUT;
            }
        } while (state != MAILBOX_STATE_IDLE);
    } else {
        state = io_read(&MAILBOX->rx_state_recv);
        if (state != MAILBOX_STATE_IDLE) {
            return MAILBOX_ERROR_AGAIN;
        }
    }

    io_write(&MAILBOX->tx_state, MAILBOX_STATE_BUSY);
    write_words(MAILBOX->tx_payload, payload, length);
    io_write(&MAILBOX->tx_length, length);
    io_write(&MAILBOX->tx_state, MAILBOX_STATE_DONE);

    counter = timeout;
    if (block_acked) {
        do {
            state = io_read(&MAILBOX->rx_state_recv);
            if (timeout != UINT32_MAX && counter-- == 0) {
                return MAILBOX_ERROR_ACK_TIMEOUT;
            }
        } while (state != MAILBOX_STATE_DONE);
    } else {
        // Transmitted successfully, but it isn't acked yet.
        return MAILBOX_ERROR_OK;
    }

    io_write(&MAILBOX->tx_state, MAILBOX_STATE_IDLE);

    // Transmitted successfully, and it has been acked.
    return MAILBOX_ERROR_OK_ACKED;
}

