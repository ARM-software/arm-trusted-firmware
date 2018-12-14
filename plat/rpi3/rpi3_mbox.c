/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <platform_def.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <lib/mmio.h>

#include "rpi3_hw.h"

/* This struct must be aligned to 16 bytes */
typedef struct __packed __aligned(16) rpi3_mbox_request {
	uint32_t	size; /* Buffer size in bytes */
	uint32_t	code; /* Request/response code */
	uint32_t	tags[0];
} rpi3_mbox_request_t;

#define RPI3_MBOX_BUFFER_SIZE		U(256)
static uint8_t __aligned(16) rpi3_mbox_buffer[RPI3_MBOX_BUFFER_SIZE];

/* Constants to perform a request/check the status of a request. */
#define RPI3_MBOX_PROCESS_REQUEST	U(0x00000000)
#define RPI3_MBOX_REQUEST_SUCCESSFUL	U(0x80000000)
#define RPI3_MBOX_REQUEST_ERROR		U(0x80000001)

/* Command constants */
#define RPI3_TAG_HARDWARE_GET_BOARD_REVISION	U(0x00010002)
#define RPI3_TAG_END				U(0x00000000)

#define RPI3_TAG_REQUEST		U(0x00000000)
#define RPI3_TAG_IS_RESPONSE		U(0x80000000) /* Set if response */
#define RPI3_TAG_RESPONSE_LENGTH_MASK	U(0x7FFFFFFF)

#define RPI3_CHANNEL_ARM_TO_VC		U(0x8)
#define RPI3_CHANNEL_MASK		U(0xF)

#define RPI3_MAILBOX_MAX_RETRIES	U(1000000)

/*******************************************************************************
 * Helpers to send requests to the VideoCore using the mailboxes.
 ******************************************************************************/
static void rpi3_vc_mailbox_request_send(void)
{
	uint32_t st, data;
	uintptr_t resp_addr, addr;
	unsigned int retries;

	/* This is the location of the request buffer */
	addr = (uintptr_t) &rpi3_mbox_buffer;

	/* Make sure that the changes are seen by the VideoCore */
	flush_dcache_range(addr, RPI3_MBOX_BUFFER_SIZE);

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
	inv_dcache_range(addr, RPI3_MBOX_BUFFER_SIZE);
}

/*******************************************************************************
 * Request board revision. Returns the revision and 0 on success, -1 on error.
 ******************************************************************************/
int rpi3_vc_hardware_get_board_revision(uint32_t *revision)
{
	uint32_t tag_request_size = sizeof(uint32_t);
	rpi3_mbox_request_t *req = (rpi3_mbox_request_t *) rpi3_mbox_buffer;

	assert(revision != NULL);

	VERBOSE("rpi3: mbox: Sending request at %p\n", (void *)req);

	req->size = sizeof(rpi3_mbox_buffer);
	req->code = RPI3_MBOX_PROCESS_REQUEST;

	req->tags[0] = RPI3_TAG_HARDWARE_GET_BOARD_REVISION;
	req->tags[1] = tag_request_size; /* Space available for the response */
	req->tags[2] = RPI3_TAG_REQUEST;
	req->tags[3] = 0; /* Placeholder for the response */

	req->tags[4] = RPI3_TAG_END;

	rpi3_vc_mailbox_request_send();

	if (req->code != RPI3_MBOX_REQUEST_SUCCESSFUL) {
		ERROR("rpi3: mbox: Code = 0x%08x\n", req->code);
		return -1;
	}

	if (req->tags[2] != (RPI3_TAG_IS_RESPONSE | tag_request_size)) {
		ERROR("rpi3: mbox: get board revision failed (0x%08x)\n",
		      req->tags[2]);
		return -1;
	}

	*revision = req->tags[3];

	return 0;
}
