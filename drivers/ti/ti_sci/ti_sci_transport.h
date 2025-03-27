/*
 * Texas Instruments SCI Transport Protocol Header
 *
 * Copyright (C) 2018-2025 Texas Instruments Incorporated - http://www.ti.com/
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef TI_SCI_TRANSPORT_H
#define TI_SCI_TRANSPORT_H

#include <stdint.h>

/**
 * enum ti_sci_transport_chan_id - Secure Proxy thread IDs
 *
 * These are the available IDs used in ti_sci_transport_{send,recv}()
 */
enum ti_sci_transport_chan_id {
#if !K3_SEC_PROXY_LITE
	RX_SECURE_TRANSPORT_CHANNEL_ID = 1,
	TX_SECURE_TRANSPORT_CHANNEL_ID,
#else
	RX_SECURE_TRANSPORT_CHANNEL_ID = 8,
	/*
	 * Note: TISCI documentation indicates "low priority", but in reality
	 * with a single thread, there is no low or high priority.. This usage
	 * is more appropriate for TF-A since we can reduce the churn as a
	 * result.
	 */
	TX_SECURE_TRANSPORT_CHANNEL_ID,
#endif /* K3_SEC_PROXY_LITE */
};

/**
 * struct ti_sci_msg - Secure proxy message structure
 * @len: Length of data in the Buffer
 * @buf: Buffer pointer
 *
 * This is the structure for data used in ti_sci_transport_{send,recv}()
 */
struct ti_sci_msg {
	size_t len;
	uint8_t *buf;
};

/**
 * ti_sci_transport_clear_rx_thread() - Clear a receive Secure Proxy thread
 * @id: Channel Identifier
 * @msg: Pointer to ti_sci_msg
 *
 * Return: 0 if all goes well, else appropriate error message
 */
int ti_sci_transport_clear_rx_thread(enum ti_sci_transport_chan_id id);

/**
 * ti_sci_transport_send() - Send data over mailbox/ Secure Proxy thread
 * @id: Channel Identifier
 * @msg: Pointer to ti_sci_msg
 *
 * Return: 0 if all goes well, else appropriate error message
 */
int ti_sci_transport_send(enum ti_sci_transport_chan_id id, const struct ti_sci_msg *msg);

/**
 * ti_sci_transport_recv() - Receive data from a Secure Proxy thread/ mailbox
 * @id: Channel Identifier
 * @msg: Pointer to ti_sci_msg
 *
 * Return: 0 if all goes well, else appropriate error message
 */
int ti_sci_transport_recv(enum ti_sci_transport_chan_id id, struct ti_sci_msg *msg);

#endif /* TI_SCI_TRANSPORT_H */
