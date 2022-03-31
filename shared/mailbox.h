#pragma once

#include <stdint.h>
#include <stdbool.h>

/*
    Cart to N64 transfer (transmit), as seen from the Cart:
        wait until {
            status == MAILBOX_STATUS_IDLE || status == MAILBOX_STATUS_RX_DONE
        }
        status          <- MAILBOX_STATUS_TX_BUSY
        memcpy(payload, payload_data, length)
        length          <- payload length
        status          <- MAILBOX_STATUS_TX_DONE
        // Optional steps below (to know when the message has been read)
        wait until {
            status == MAILBOX_STATUS_RX_DONE
        }
        status          <- MAILBOX_STATUS_IDLE

    Cart to N64 transfer (receive), as seen from the N64:
        wait until {
            status == MAILBOX_STATUS_TX_DONE
        }
        status          <- MAILBOX_STATUS_RX_BUSY
        received_length <- length
        memcpy(received_payload, payload, received_length)
        status          <- MAILBOX_STATUS_RX_DONE
        // Optional step below (to know when the ack has been read)
        wait until {
            status == MAILBOX_STATUS_IDLE
        }
*/

#define MAILBOX_PAYLOAD_SIZE (64)

typedef enum {
    MAILBOX_STATUS_IDLE    = (1 << 0), // Transmitter is ready to write, Receiver must not read
    MAILBOX_STATUS_TX_BUSY = (1 << 1), // Transmitter is writing,        Receiver must not read
    MAILBOX_STATUS_TX_DONE = (1 << 2), // Transmitter is done writing,   Receiver is ready to read
    MAILBOX_STATUS_RX_BUSY = (1 << 3), // Transmitter must not write,    Receiver is reading
    MAILBOX_STATUS_RX_DONE = (1 << 4), // Transmitter must not write,    Receiver is done reading
} mailbox_status_t;

typedef struct {
    uint32_t status; // mailbox_status_t
    uint32_t length;
    uint8_t  payload[MAILBOX_PAYLOAD_SIZE];
} mailbox_msg_t;

#define MAILBOX_BASE (0x10f00000)

// Cart to N64 (Incoming)
#define MAILBOX0 ((volatile mailbox_t *) (MAILBOX_BASE))

// N64 to Cart (Outgoing)
#define MAILBOX1 ((volatile mailbox_t *) (MAILBOX_BASE + sizeof(mailbox_msg_t)))

/**
 * @brief      Receives a payload from the mailbox.
 *
 * @param      block  Set to `true` for blocking operation.
 * @param[out] buf    Receive buffer. Must be at least MAILBOX_PAYLOAD_SIZE bytes large.
 * @return     Length or error code
 *             - -1, No pending payload. Only if \p block is `false`.
 *             - >=0, Length of the received payload.
 */
int32_t mailbox_rx(bool block, uint8_t *buf);

/**
 * @brief      Transmits a payload to the mailbox.
 *
 * @param      block_outgoing  Set to `true` to block until transmit is possible.
 * @param      block_received  Set to `true` to block until payload has been received.
 * @param[in]  buf             Transmit buffer.
 * @param      length          Length of transmit buffer. Must not be larger than MAILBOX_PAYLOAD_SIZE bytes.
 * @return     Error code
 *             - -1, Mailbox busy (receiver is reading). Only if \p block_outgoing is `false`.
 *             -  0, Message transmitted but not read. Only if \p block_received is `false`.
 *             -  1, Message transmitted and read.
 */
int32_t mailbox_tx(bool block_outgoing, bool block_received, uint8_t *buf, uint32_t length);

