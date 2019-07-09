/*
 * Copyright (c) 2018-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <platform_def.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <lib/mmio.h>

#include <rpi_hw.h>

#include <drivers/rpi3/mailbox/rpi3_mbox.h>

#define RPI3_MAILBOX_MAX_RETRIES	U(1000000)

/*******************************************************************************
 * Routine to send requests to the VideoCore using the mailboxes.
 ******************************************************************************/
void rpi3_vc_mailbox_request_send(rpi3_mbox_request_t *req, int req_size)
{
	uint32_t st, data;
	uintptr_t resp_addr, addr;
	unsigned int retries;

	/* This is the location of the request buffer */
	addr = (uintptr_t)req;

	/* Make sure that the changes are seen by the VideoCore */
	flush_dcache_range(addr, req_size);

	/* Wait until the outbound mailbox is empty */
	retries = 0U;

	do {
		st = mmio_read_32(RPI3_MBOX_BASE + RPI3_MBOX1_STATUS_OFFSET);

		retries++;
		if (retries == RPI3_MAILBOX_MAX_RETRIES) {
			ERROR("rpi3: mbox: Send request timeout\n");
			return;
		}

	} while ((st & RPI3_MBOX_STATUS_EMPTY_MASK) == 0U);

	/* Send base address of this message to start request */
	mmio_write_32(RPI3_MBOX_BASE + RPI3_MBOX1_WRITE_OFFSET,
		      RPI3_CHANNEL_ARM_TO_VC | (uint32_t) addr);

	/* Wait until the inbound mailbox isn't empty */
	retries = 0U;

	do {
		st = mmio_read_32(RPI3_MBOX_BASE + RPI3_MBOX0_STATUS_OFFSET);

		retries++;
		if (retries == RPI3_MAILBOX_MAX_RETRIES) {
			ERROR("rpi3: mbox: Receive response timeout\n");
			return;
		}

	} while ((st & RPI3_MBOX_STATUS_EMPTY_MASK) != 0U);

	/* Get location and channel */
	data = mmio_read_32(RPI3_MBOX_BASE + RPI3_MBOX0_READ_OFFSET);

	if ((data & RPI3_CHANNEL_MASK) != RPI3_CHANNEL_ARM_TO_VC) {
		ERROR("rpi3: mbox: Wrong channel: 0x%08x\n", data);
		panic();
	}

	resp_addr = (uintptr_t)(data & ~RPI3_CHANNEL_MASK);
	if (addr != resp_addr) {
		ERROR("rpi3: mbox: Unexpected address: 0x%08x\n", data);
		panic();
	}

	/* Make sure that the data seen by the CPU is up to date */
	inv_dcache_range(addr, req_size);
}
