/*
 * Copyright (c) 2014, ARM Limited and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <arch_helpers.h>
#include <platform.h>
#include "juno_def.h"
#include "mhu.h"
#include "scp_bootloader.h"
#include "scpi.h"

/* Boot commands sent from AP -> SCP */
#define BOOT_CMD_START	0x01
#define BOOT_CMD_DATA	0x02

typedef struct {
	uint32_t image_size;
} cmd_start_payload;

typedef struct {
	uint32_t sequence_num;
	uint32_t offset;
	uint32_t size;
} cmd_data_payload;

#define BOOT_DATA_MAX_SIZE  0x1000

/* Boot commands sent from SCP -> AP */
#define BOOT_CMD_ACK	0x03
#define BOOT_CMD_NACK	0x04

typedef struct {
	uint32_t sequence_num;
} cmd_ack_payload;

/*
 * Unlike the runtime protocol, the boot protocol uses the same memory region
 * for both AP -> SCP and SCP -> AP transfers; define the address of this...
 */
static void * const cmd_payload = (void *)(MHU_SECURE_BASE + 0x0080);

static void *scp_boot_message_start(void)
{
	mhu_secure_message_start();

	return cmd_payload;
}

static void scp_boot_message_send(unsigned command, size_t size)
{
	/* Make sure payload can be seen by SCP */
	if (MHU_PAYLOAD_CACHED)
		flush_dcache_range((unsigned long)cmd_payload, size);

	/* Send command to SCP */
	mhu_secure_message_send(command | (size << 8));
}

static uint32_t scp_boot_message_wait(size_t size)
{
	uint32_t response =  mhu_secure_message_wait();

	/* Make sure we see the reply from the SCP and not any stale data */
	if (MHU_PAYLOAD_CACHED)
		inv_dcache_range((unsigned long)cmd_payload, size);

	return response & 0xff;
}

static void scp_boot_message_end(void)
{
	mhu_secure_message_end();
}

static int transfer_block(uint32_t sequence_num, uint32_t offset, uint32_t size)
{
	cmd_data_payload *cmd_data = scp_boot_message_start();
	cmd_data->sequence_num = sequence_num;
	cmd_data->offset = offset;
	cmd_data->size = size;

	scp_boot_message_send(BOOT_CMD_DATA, sizeof(*cmd_data));

	cmd_ack_payload *cmd_ack = cmd_payload;
	int ok = scp_boot_message_wait(sizeof(*cmd_ack)) == BOOT_CMD_ACK
		 && cmd_ack->sequence_num == sequence_num;

	scp_boot_message_end();

	return ok;
}

int scp_bootloader_transfer(void *image, unsigned int image_size)
{
	uintptr_t offset = (uintptr_t)image - MHU_SECURE_BASE;
	uintptr_t end = offset + image_size;
	uint32_t response;

	mhu_secure_init();

	/* Initiate communications with SCP */
	do {
		cmd_start_payload *cmd_start = scp_boot_message_start();
		cmd_start->image_size = image_size;

		scp_boot_message_send(BOOT_CMD_START, sizeof(*cmd_start));

		response = scp_boot_message_wait(0);

		scp_boot_message_end();
	} while (response != BOOT_CMD_ACK);

	/* Transfer image to SCP a block at a time */
	uint32_t sequence_num = 1;
	size_t size;
	while ((size = end - offset) != 0) {
		if (size > BOOT_DATA_MAX_SIZE)
			size = BOOT_DATA_MAX_SIZE;
		while (!transfer_block(sequence_num, offset, size))
			; /* Retry forever */
		offset += size;
		sequence_num++;
	}

	/* Wait for SCP to signal it's ready */
	return scpi_wait_ready();
}
