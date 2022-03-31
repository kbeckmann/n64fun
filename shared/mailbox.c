#include "mailbox.h"

#include <string.h>

#define MAILBOX_BASE (0x10f00000)

// Cart to N64 (Incoming)
#define MAILBOX_RX ((volatile mailbox_t *) (MAILBOX_BASE))

// N64 to Cart (Outgoing)
#define MAILBOX_TX ((volatile mailbox_t *) (MAILBOX_BASE + sizeof(mailbox_t)))

static void read_words(uint32_t *dest, volatile uint32_t *src, size_t words)
{
    while (words--) {
        *dest++ = *src++;
    }
}

static void write_words(volatile uint32_t *dest, uint32_t *src, size_t words)
{
    while (words--) {
        *dest++ = *src++;
    }
}

int32_t mailbox_rx(bool block_ready, bool block_acked, uint32_t *payload)
{
    mailbox_status_t status;

    if (block_ready) {
        do {
            status = MAILBOX_RX->status;
        } while (status != MAILBOX_STATUS_TX_DONE);
    } else {
        status = MAILBOX_RX->status;
        if (status != MAILBOX_STATUS_TX_DONE) {
            return -1;
        }
    }

    MAILBOX_RX->status = MAILBOX_STATUS_RX_BUSY;
    uint32_t length = MAILBOX_RX->length;
    read_words(payload, MAILBOX_RX->payload, length);
    MAILBOX_RX->status = MAILBOX_STATUS_RX_DONE;

    if (block_acked) {
        do {
            status = MAILBOX_RX->status;
        } while (status != MAILBOX_STATUS_IDLE);
    }

    return length;
}

int32_t mailbox_tx(bool block_ready, bool block_acked, uint32_t *payload, uint32_t length)
{
    mailbox_status_t status;

    if (block_ready) {
        do {
            status = MAILBOX_TX->status;
        } while ((status != MAILBOX_STATUS_IDLE) && (status != MAILBOX_STATUS_RX_DONE));
    } else {
        status = MAILBOX_TX->status;
        if ((status != MAILBOX_STATUS_IDLE) && (status != MAILBOX_STATUS_RX_DONE)) {
            return -1;
        }
    }

    MAILBOX_TX->status = MAILBOX_STATUS_TX_BUSY;
    write_words(MAILBOX_TX->payload, payload, length);
    MAILBOX_TX->length = length;
    MAILBOX_TX->status = MAILBOX_STATUS_TX_DONE;

    if (block_acked) {
        do {
            status = MAILBOX_TX->status;
        } while (status == MAILBOX_STATUS_RX_DONE);
    } else {
        // Transmitted successfully, but it isn't acked yet.
        return 0;
    }

    MAILBOX_TX->status = MAILBOX_STATUS_IDLE;

    // Transmitted successfully, and it has been acked.
    return 1;
}

