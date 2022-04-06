#pragma once

#include <stdint.h>
#include <stdbool.h>

/*
    TX: N64 to Cart transfer, as seen from the N64:
        tx_state       <- MAILBOX_STATE_IDLE
        wait until rx_state_recv == MAILBOX_STATE_IDLE

        tx_state       <- MAILBOX_STATE_BUSY
        memcpy(tx_payload, payload_data, length)
        tx_length      <- payload length
        tx_state       <- MAILBOX_STATE_DONE

        wait until rx_state_recv == MAILBOX_STATE_DONE
        tx_state       <- MAILBOX_STATE_IDLE

    RX: Cart to N64 transfer, as seen from the N64:
        tx_state_recv  <- MAILBOX_STATE_IDLE
        wait until rx_state == MAILBOX_STATE_IDLE

        tx_state_recv  <- MAILBOX_STATE_BUSY
        received_length <- rx_length
        memcpy(received_payload, rx_payload, received_length)
        tx_state_recv  <- MAILBOX_STATE_DONE

        wait until rx_state == MAILBOX_STATE_DONE
        tx_state_recv  <- MAILBOX_STATE_IDLE
*/

#define MAILBOX_PAYLOAD_WORDS (61)

typedef enum {
    MAILBOX_ERROR_ACK_TIMEOUT  = -3,
    MAILBOX_ERROR_IDLE_TIMEOUT = -2,
    MAILBOX_ERROR_AGAIN        = -1,
    MAILBOX_ERROR_OK           = 0,
    MAILBOX_ERROR_OK_ACKED     = 1,
} mailbox_error_t;

typedef enum {
    MAILBOX_STATE_IDLE    = 0,
    MAILBOX_STATE_BUSY    = 1,
    MAILBOX_STATE_DONE    = 2,
} mailbox_state_t;

typedef struct {
    uint32_t tx_state;                             // W mailbox_state_t
    uint32_t tx_state_recv;                        // W mailbox_state_t
    uint32_t tx_length;                            // W
    uint32_t tx_payload[MAILBOX_PAYLOAD_WORDS];    // W

    // +0x100 offset
    uint32_t rx_state;                             // R mailbox_state_t
    uint32_t rx_state_recv;                        // R mailbox_state_t
    uint32_t rx_length;                            // R
    uint32_t rx_payload[MAILBOX_PAYLOAD_WORDS];    // R
} mailbox_t;

/**
 * @brief      Initializes the mailbox.
 */
void mailbox_init(void);

/**
 * @brief      Receives a payload from the mailbox.
 *
 * @param[out] payload      Receive buffer. Buffer must be 32 bit aligned.
 * @param      block_ready  Set to `true` to block until receive is possible.
 * @param      block_acked  Set to `true` to block until transmitter has acked our ack.
 *                          Must be at least MAILBOX_PAYLOAD_WORDS words large.
 * @param      timeout      Max number of cycles for blocking operations.
 *                          Set to UINT32_MAX to block indefinitely.
 * @return     Length or error code
 *             - -1, No pending payload. Only if \p block_ready is `false`.
 *             - >=0, Length of the received payload.
 */
int32_t mailbox_rx(uint32_t *payload, bool block_ready, bool block_acked, uint32_t timeout);

/**
 * @brief      Transmits a payload to the mailbox.
 *
 * @param[in]  payload      Transmit buffer. Buffer must be 32 bit aligned.
 * @param      length       Number of 32-bit words to transmit. Must not be larger than MAILBOX_PAYLOAD_WORDS.
 * @param      block_ready  Set to `true` to block until transmit is possible.
 * @param      block_acked  Set to `true` to block until payload has been acked by the receiver.
 * @param      timeout      Max number of cycles for blocking operations.
 *                          Set to UINT32_MAX to block indefinitely.
 * @return     Error code
 *             - -1, Mailbox busy (receiver is reading). Only if \p block_ready is `false`.
 *             -  0, Message transmitted but not acked. Only if \p block_acked is `false`.
 *             -  1, Message transmitted and acked.
 */
int32_t mailbox_tx(uint32_t *payload, uint32_t length, bool block_ready, bool block_acked, uint32_t timeout);

