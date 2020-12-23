/*
 * Texas Instruments K3 Secure Proxy Driver
 *   Based on Linux and U-Boot implementation
 *
 * Copyright (C) 2018 Texas Instruments Incorporated - http://www.ti.com/
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SEC_PROXY_H
#define SEC_PROXY_H

#include <stdint.h>

/**
 * enum k3_sec_proxy_chan_id - Secure Proxy thread IDs
 *
 * These the available IDs used in k3_sec_proxy_{send,recv}()
 * There are two schemes we use:
 * * if K3_SEC_PROXY_LITE = 1, we just have two threads to talk
 * * if K3_SEC_PROXY_LITE = 0, we have the full fledged
 *   communication scheme available.
 */
enum k3_sec_proxy_chan_id {
#if !K3_SEC_PROXY_LITE
	SP_NOTIFY = 0,
	SP_RESPONSE,
	SP_HIGH_PRIORITY,
	SP_LOW_PRIORITY,
	SP_NOTIFY_RESP,
#else
	SP_RESPONSE = 8,
	/*
	 * Note: TISCI documentation indicates "low priority", but in reality
	 * with a single thread, there is no low or high priority.. This usage
	 * is more appropriate for TF-A since we can reduce the churn as a
	 * result.
	 */
	SP_HIGH_PRIORITY,
#endif /* K3_SEC_PROXY_LITE */
};

/**
 * struct k3_sec_proxy_msg - Secure proxy message structure
 * @len: Length of data in the Buffer
 * @buf: Buffer pointer
 *
 * This is the structure for data used in k3_sec_proxy_{send,recv}()
 */
struct k3_sec_proxy_msg {
	size_t len;
	uint8_t *buf;
};

/**
 * k3_sec_proxy_send() - Send data over a Secure Proxy thread
 * @id: Channel Identifier
 * @msg: Pointer to k3_sec_proxy_msg
 *
 * Return: 0 if all goes well, else appropriate error message
 */
int k3_sec_proxy_clear_rx_thread(enum k3_sec_proxy_chan_id id);

/**
 * k3_sec_proxy_send() - Send data over a Secure Proxy thread
 * @id: Channel Identifier
 * @msg: Pointer to k3_sec_proxy_msg
 *
 * Return: 0 if all goes well, else appropriate error message
 */
int k3_sec_proxy_send(enum k3_sec_proxy_chan_id id, const struct k3_sec_proxy_msg *msg);

/**
 * k3_sec_proxy_recv() - Receive data from a Secure Proxy thread
 * @id: Channel Identifier
 * @msg: Pointer to k3_sec_proxy_msg
 *
 * Return: 0 if all goes well, else appropriate error message
 */
int k3_sec_proxy_recv(enum k3_sec_proxy_chan_id id, struct k3_sec_proxy_msg *msg);

#endif /* SEC_PROXY_H */
