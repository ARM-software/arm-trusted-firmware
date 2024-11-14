/*
 * Texas Instruments Mailbox Driver
 *
 * Copyright (C) 2024 Texas Instruments Incorporated - https://www.ti.com/
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

#include "mailbox.h"

int ti_mailbox_poll_status(void)
{
	int i = 0;
	uint32_t tick_start = (uint32_t)read_cntpct_el0();
	uint32_t ticks_per_us = SYS_COUNTER_FREQ_IN_TICKS / 1000000;

	while (!i) {
		i = mmio_read_32(TIFS_MAILBOX_BASE1 + TIFS_MAILBOX_MSG_STATUS);
		if (((uint32_t)read_cntpct_el0() - tick_start) >
		    (SEC_PROXY_TIMEOUT_US * ticks_per_us)) {
			return -ETIMEDOUT;
		}
	}
	return 0;
}

void ti_boot_notification(void)
{
	uint64_t rcv_addr;

	if (ti_mailbox_poll_status()) {
		ERROR("Timeout waiting for boot notification\n");
		return;
	}

	/* consume boot notification */
	rcv_addr = mmio_read_32(TIFS_MAILBOX_BASE1 + TIFS_MAILBOX_MSG);
	VERBOSE("%s: boot notification received from TIFS: 0x%lx\n", __func__, rcv_addr);
}

int ti_sci_transport_clear_rx_thread(enum k3_sec_proxy_chan_id id)
{
	/* Dummy function to maintain API backward compatibility */

	return 0;
}

int ti_sci_transport_send(enum k3_sec_proxy_chan_id id, const struct ti_sci_msg *msg)
{
	int num_bytes;
	void *dst_ptr = (void *)AM62L_RSVD_SRAM_BASE;

	num_bytes = msg->len / sizeof(uint8_t);

	/* move the buffer contents into the SRAM to be accessed by TIFS */
	memmove(dst_ptr, msg->buf, num_bytes);

	mmio_write_32(TIFS_MAILBOX_BASE0 + TIFS_MAILBOX_MSG, (unsigned long)(void *)dst_ptr);

	return 0;
}

int ti_sci_transport_recv(enum k3_sec_proxy_chan_id id, struct ti_sci_msg *msg)
{
	int num_bytes;
	uint64_t rcv_addr;

	if (ti_mailbox_poll_status()) {
		ERROR("Timeout waiting for receive\n");
		return -ETIMEDOUT;
	}

	rcv_addr = mmio_read_32(TIFS_MAILBOX_BASE1 + TIFS_MAILBOX_MSG);
	num_bytes = msg->len / sizeof(uint8_t);

	for (int i = 0; i < num_bytes; i++) {
		((uint8_t *)msg->buf)[i] = *(uint8_t *)(rcv_addr);
		rcv_addr += sizeof(uint8_t);
	}

	return 0;
}
