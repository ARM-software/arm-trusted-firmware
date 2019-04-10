/*
 * Texas Instruments K3 Secure Proxy Driver
 *   Based on Linux and U-Boot implementation
 *
 * Copyright (C) 2018 Texas Instruments Incorporated - http://www.ti.com/
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>
#include <stdlib.h>

#include <platform_def.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <lib/mmio.h>
#include <lib/utils.h>
#include <lib/utils_def.h>

#include "sec_proxy.h"

/* SEC PROXY RT THREAD STATUS */
#define RT_THREAD_STATUS			(0x0)
#define RT_THREAD_STATUS_ERROR_SHIFT		(31)
#define RT_THREAD_STATUS_ERROR_MASK		BIT(31)
#define RT_THREAD_STATUS_CUR_CNT_SHIFT		(0)
#define RT_THREAD_STATUS_CUR_CNT_MASK		GENMASK(7, 0)

/* SEC PROXY SCFG THREAD CTRL */
#define SCFG_THREAD_CTRL			(0x1000)
#define SCFG_THREAD_CTRL_DIR_SHIFT		(31)
#define SCFG_THREAD_CTRL_DIR_MASK		BIT(31)

#define SEC_PROXY_THREAD(base, x)		((base) + (0x1000 * (x)))
#define THREAD_IS_RX				(1)
#define THREAD_IS_TX				(0)

/**
 * struct k3_sec_proxy_desc - Description of secure proxy integration
 * @timeout_us:		Timeout for communication (in Microseconds)
 * @max_msg_size:	Message size in bytes
 * @data_start_offset:	Offset of the First data register of the thread
 * @data_end_offset:	Offset of the Last data register of the thread
 */
struct k3_sec_proxy_desc {
	uint32_t timeout_us;
	uint16_t max_msg_size;
	uint16_t data_start_offset;
	uint16_t data_end_offset;
};

/**
 * struct k3_sec_proxy_thread - Description of a Secure Proxy Thread
 * @name:	Thread Name
 * @data:	Thread Data path region for target
 * @scfg:	Secure Config Region for Thread
 * @rt:		RealTime Region for Thread
 */
struct k3_sec_proxy_thread {
	const char *name;
	uintptr_t data;
	uintptr_t scfg;
	uintptr_t rt;
};

/**
 * struct k3_sec_proxy_mbox - Description of a Secure Proxy Instance
 * @desc:	Description of the SoC integration
 * @chans:	Array for valid thread instances
 */
struct k3_sec_proxy_mbox {
	const struct k3_sec_proxy_desc desc;
	struct k3_sec_proxy_thread threads[];
};

/*
 * Thread ID #0: DMSC notify
 * Thread ID #1: DMSC request response
 * Thread ID #2: DMSC request high priority
 * Thread ID #3: DMSC request low priority
 * Thread ID #4: DMSC notify response
 */
#define SP_THREAD(_x) \
	[_x] = { \
		.name = #_x, \
		.data = SEC_PROXY_THREAD(SEC_PROXY_DATA_BASE, _x), \
		.scfg = SEC_PROXY_THREAD(SEC_PROXY_SCFG_BASE, _x), \
		.rt = SEC_PROXY_THREAD(SEC_PROXY_RT_BASE, _x), \
	}

static struct k3_sec_proxy_mbox spm = {
	.desc = {
		.timeout_us = SEC_PROXY_TIMEOUT_US,
		.max_msg_size = SEC_PROXY_MAX_MESSAGE_SIZE,
		.data_start_offset = 0x4,
		.data_end_offset = 0x3C,
	},
	.threads = {
		SP_THREAD(SP_NOTIFY),
		SP_THREAD(SP_RESPONSE),
		SP_THREAD(SP_HIGH_PRIORITY),
		SP_THREAD(SP_LOW_PRIORITY),
		SP_THREAD(SP_NOTIFY_RESP),
	},
};

/**
 * struct sec_msg_hdr - Message header for secure messages and responses
 * @checksum:	CRC of message for integrity checking
 */
union sec_msg_hdr {
	struct {
		uint16_t checksum;
		uint16_t reserved;
	} __packed;
	uint32_t data;
};

/**
 * k3_sec_proxy_verify_thread() - Verify thread status before
 *				  sending/receiving data
 * @spt: Pointer to Secure Proxy thread description
 * @dir: Direction of the thread
 *
 * Return: 0 if all goes well, else appropriate error message
 */
static inline int k3_sec_proxy_verify_thread(struct k3_sec_proxy_thread *spt,
					     uint32_t dir)
{
	/* Check for any errors already available */
	if (mmio_read_32(spt->rt + RT_THREAD_STATUS) &
	    RT_THREAD_STATUS_ERROR_MASK) {
		ERROR("Thread %s is corrupted, cannot send data\n", spt->name);
		return -EINVAL;
	}

	/* Make sure thread is configured for right direction */
	if ((mmio_read_32(spt->scfg + SCFG_THREAD_CTRL) & SCFG_THREAD_CTRL_DIR_MASK)
	    != (dir << SCFG_THREAD_CTRL_DIR_SHIFT)) {
		if (dir == THREAD_IS_TX)
			ERROR("Trying to send data on RX Thread %s\n",
			      spt->name);
		else
			ERROR("Trying to receive data on TX Thread %s\n",
			      spt->name);
		return -EINVAL;
	}

	/* Check the message queue before sending/receiving data */
	uint32_t tick_start = (uint32_t)read_cntpct_el0();
	uint32_t ticks_per_us = SYS_COUNTER_FREQ_IN_TICKS / 1000000;
	while (!(mmio_read_32(spt->rt + RT_THREAD_STATUS) & RT_THREAD_STATUS_CUR_CNT_MASK)) {
		VERBOSE("Waiting for thread %s to %s\n",
			spt->name, (dir == THREAD_IS_TX) ? "empty" : "fill");
		if (((uint32_t)read_cntpct_el0() - tick_start) >
		    (spm.desc.timeout_us * ticks_per_us)) {
			ERROR("Timeout waiting for thread %s to %s\n",
				spt->name, (dir == THREAD_IS_TX) ? "empty" : "fill");
			return -ETIMEDOUT;
		}
	}

	return 0;
}

/**
 * k3_sec_proxy_clear_rx_thread() - Clear Secure Proxy thread
 *
 * @id: Channel Identifier
 *
 * Return: 0 if all goes well, else appropriate error message
 */
int k3_sec_proxy_clear_rx_thread(enum k3_sec_proxy_chan_id id)
{
	struct k3_sec_proxy_thread *spt = &spm.threads[id];

	/* Check for any errors already available */
	if (mmio_read_32(spt->rt + RT_THREAD_STATUS) &
	    RT_THREAD_STATUS_ERROR_MASK) {
		ERROR("Thread %s is corrupted, cannot send data\n", spt->name);
		return -EINVAL;
	}

	/* Make sure thread is configured for right direction */
	if (!(mmio_read_32(spt->scfg + SCFG_THREAD_CTRL) & SCFG_THREAD_CTRL_DIR_MASK)) {
		ERROR("Cannot clear a transmit thread %s\n", spt->name);
		return -EINVAL;
	}

	/* Read off messages from thread until empty */
	uint32_t try_count = 10;
	while (mmio_read_32(spt->rt + RT_THREAD_STATUS) & RT_THREAD_STATUS_CUR_CNT_MASK) {
		if (!(try_count--)) {
			ERROR("Could not clear all messages from thread %s\n", spt->name);
			return -ETIMEDOUT;
		}
		WARN("Clearing message from thread %s\n", spt->name);
		mmio_read_32(spt->data + spm.desc.data_end_offset);
	}

	return 0;
}

/**
 * k3_sec_proxy_send() - Send data over a Secure Proxy thread
 * @id: Channel Identifier
 * @msg: Pointer to k3_sec_proxy_msg
 *
 * Return: 0 if all goes well, else appropriate error message
 */
int k3_sec_proxy_send(enum k3_sec_proxy_chan_id id, const struct k3_sec_proxy_msg *msg)
{
	struct k3_sec_proxy_thread *spt = &spm.threads[id];
	union sec_msg_hdr secure_header;
	int num_words, trail_bytes, i, ret;
	uintptr_t data_reg;

	ret = k3_sec_proxy_verify_thread(spt, THREAD_IS_TX);
	if (ret) {
		ERROR("Thread %s verification failed (%d)\n", spt->name, ret);
		return ret;
	}

	/* Check the message size */
	if (msg->len + sizeof(secure_header) > spm.desc.max_msg_size) {
		ERROR("Thread %s message length %lu > max msg size\n",
		      spt->name, msg->len);
		return -EINVAL;
	}

	/* TODO: Calculate checksum */
	secure_header.checksum = 0;

	/* Send the secure header */
	data_reg = spm.desc.data_start_offset;
	mmio_write_32(spt->data + data_reg, secure_header.data);
	data_reg += sizeof(uint32_t);

	/* Send whole words */
	num_words = msg->len / sizeof(uint32_t);
	for (i = 0; i < num_words; i++) {
		mmio_write_32(spt->data + data_reg, ((uint32_t *)msg->buf)[i]);
		data_reg += sizeof(uint32_t);
	}

	/* Send remaining bytes */
	trail_bytes = msg->len % sizeof(uint32_t);
	if (trail_bytes) {
		uint32_t data_trail = 0;

		i = msg->len - trail_bytes;
		while (trail_bytes--) {
			data_trail <<= 8;
			data_trail |= msg->buf[i++];
		}

		mmio_write_32(spt->data + data_reg, data_trail);
		data_reg += sizeof(uint32_t);
	}
	/*
	 * 'data_reg' indicates next register to write. If we did not already
	 * write on tx complete reg(last reg), we must do so for transmit
	 */
	if (data_reg <= spm.desc.data_end_offset)
		mmio_write_32(spt->data + spm.desc.data_end_offset, 0);

	VERBOSE("Message successfully sent on thread %s\n", spt->name);

	return 0;
}

/**
 * k3_sec_proxy_recv() - Receive data from a Secure Proxy thread
 * @id: Channel Identifier
 * @msg: Pointer to k3_sec_proxy_msg
 *
 * Return: 0 if all goes well, else appropriate error message
 */
int k3_sec_proxy_recv(enum k3_sec_proxy_chan_id id, struct k3_sec_proxy_msg *msg)
{
	struct k3_sec_proxy_thread *spt = &spm.threads[id];
	union sec_msg_hdr secure_header;
	uintptr_t data_reg;
	int num_words, trail_bytes, i, ret;

	ret = k3_sec_proxy_verify_thread(spt, THREAD_IS_RX);
	if (ret) {
		ERROR("Thread %s verification failed (%d)\n", spt->name, ret);
		return ret;
	}

	/* Read secure header */
	data_reg = spm.desc.data_start_offset;
	secure_header.data = mmio_read_32(spt->data + data_reg);
	data_reg += sizeof(uint32_t);

	/* Read whole words */
	num_words = msg->len / sizeof(uint32_t);
	for (i = 0; i < num_words; i++) {
		((uint32_t *)msg->buf)[i] = mmio_read_32(spt->data + data_reg);
		data_reg += sizeof(uint32_t);
	}

	/* Read remaining bytes */
	trail_bytes = msg->len % sizeof(uint32_t);
	if (trail_bytes) {
		uint32_t data_trail = mmio_read_32(spt->data + data_reg);
		data_reg += sizeof(uint32_t);

		i = msg->len - trail_bytes;
		while (trail_bytes--) {
			msg->buf[i] = data_trail & 0xff;
			data_trail >>= 8;
		}
	}

	/*
	 * 'data_reg' indicates next register to read. If we did not already
	 * read on rx complete reg(last reg), we must do so for receive
	 */
	if (data_reg <= spm.desc.data_end_offset)
		mmio_read_32(spt->data + spm.desc.data_end_offset);

	/* TODO: Verify checksum */
	(void)secure_header.checksum;

	VERBOSE("Message successfully received from thread %s\n", spt->name);

	return 0;
}
