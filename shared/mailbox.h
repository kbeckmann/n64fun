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

#define MAILBOX_PAYLOAD_WORDS (64)

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
    uint32_t payload[MAILBOX_PAYLOAD_WORDS];
} mailbox_t;

/**
 * @brief      Receives a payload from the mailbox.
 *
 * @param      block_ready  Set to `true` to block until receive is possible.
 * @param      block_acked  Set to `true` to block until transmitter has acked our ack.
 * @param[out] payload      Receive buffer. Buffer must be 32 bit aligned.
 *                          Must be at least MAILBOX_PAYLOAD_WORDS words large.
 * @return     Length or error code
 *             - -1, No pending payload. Only if \p block_ready is `false`.
 *             - >=0, Length of the received payload.
 */
int32_t mailbox_rx(bool block_ready, bool block_acked, uint32_t *payload);

/**
 * @brief      Transmits a payload to the mailbox.
 *
 * @param      block_ready  Set to `true` to block until transmit is possible.
 * @param      block_acked  Set to `true` to block until payload has been acked by the receiver.
 * @param[in]  payload      Transmit buffer. Buffer must be 32 bit aligned.
 * @param      length       Number of 32-bit words to transmit. Must not be larger than MAILBOX_PAYLOAD_WORDS.
 * @return     Error code
 *             - -1, Mailbox busy (receiver is reading). Only if \p block_ready is `false`.
 *             -  0, Message transmitted but not acked. Only if \p block_acked is `false`.
 *             -  1, Message transmitted and acked.
 */
int32_t mailbox_tx(bool block_ready, bool block_acked, uint32_t *payload, uint32_t length);

