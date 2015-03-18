/*
 * Copyright (c) 2014-2015, ARM Limited and Contributors. All rights reserved.
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
#include <assert.h>
#include <debug.h>
#include <platform.h>
#include <platform_def.h>
#include <stdint.h>
#include "juno_def.h"
#include "mhu.h"
#include "scp_bootloader.h"
#include "scpi.h"

/* ID of the MHU slot used for the BOM protocol */
#define BOM_MHU_SLOT_ID		0

/* Boot commands sent from AP -> SCP */
#define BOOT_CMD_INFO	0x00
#define BOOT_CMD_DATA	0x01

/* BOM command header */
typedef struct {
	uint32_t id : 8;
	uint32_t reserved : 24;
} bom_cmd_t;

typedef struct {
	uint32_t image_size;
	uint32_t checksum;
} cmd_info_payload_t;

/*
 * Unlike the SCPI protocol, the boot protocol uses the same memory region
 * for both AP -> SCP and SCP -> AP transfers; define the address of this...
 */
#define BOM_SHARED_MEM		(MHU_SECURE_BASE + 0x0080)
#define BOM_CMD_HEADER		((bom_cmd_t *) BOM_SHARED_MEM)
#define BOM_CMD_PAYLOAD		((void *) (BOM_SHARED_MEM + sizeof(bom_cmd_t)))

typedef struct {
	/* Offset from the base address of the Trusted RAM */
	uint32_t offset;
	uint32_t block_size;
} cmd_data_payload_t;

static void scp_boot_message_start(void)
{
	mhu_secure_message_start(BOM_MHU_SLOT_ID);
}

static void scp_boot_message_send(size_t payload_size)
{
	/* Make sure payload can be seen by SCP */
	if (MHU_PAYLOAD_CACHED)
		flush_dcache_range(BOM_SHARED_MEM,
				   sizeof(bom_cmd_t) + payload_size);

	/* Send command to SCP */
	mhu_secure_message_send(BOM_MHU_SLOT_ID);
}

static uint32_t scp_boot_message_wait(size_t size)
{
	uint32_t mhu_status;

	mhu_status = mhu_secure_message_wait();

	/* Expect an SCP Boot Protocol message, reject any other protocol */
	if (mhu_status != (1 << BOM_MHU_SLOT_ID)) {
		ERROR("MHU: Unexpected protocol (MHU status: 0x%x)\n",
			mhu_status);
		panic();
	}

	/* Make sure we see the reply from the SCP and not any stale data */
	if (MHU_PAYLOAD_CACHED)
		inv_dcache_range(BOM_SHARED_MEM, size);

	return *(uint32_t *) BOM_SHARED_MEM;
}

static void scp_boot_message_end(void)
{
	mhu_secure_message_end(BOM_MHU_SLOT_ID);
}

int scp_bootloader_transfer(void *image, unsigned int image_size)
{
	uint32_t response;
	uint32_t checksum;
	cmd_info_payload_t *cmd_info_payload;
	cmd_data_payload_t *cmd_data_payload;

	assert((uintptr_t) image == BL30_BASE);

	if ((image_size == 0) || (image_size % 4 != 0)) {
		ERROR("Invalid size for the BL3-0 image. Must be a multiple of "
			"4 bytes and not zero (current size = 0x%x)\n",
			image_size);
		return -1;
	}

	/* Extract the checksum from the image */
	checksum = *(uint32_t *) image;
	image = (char *) image + sizeof(checksum);
	image_size -= sizeof(checksum);

	mhu_secure_init();

	VERBOSE("Send info about the BL3-0 image to be transferred to SCP\n");

	/*
	 * Send information about the SCP firmware image about to be transferred
	 * to SCP
	 */
	scp_boot_message_start();

	BOM_CMD_HEADER->id = BOOT_CMD_INFO;
	cmd_info_payload = BOM_CMD_PAYLOAD;
	cmd_info_payload->image_size = image_size;
	cmd_info_payload->checksum = checksum;

	scp_boot_message_send(sizeof(*cmd_info_payload));
	response = scp_boot_message_wait(sizeof(response));
	scp_boot_message_end();

	if (response != 0) {
		ERROR("SCP BOOT_CMD_INFO returned error %u\n", response);
		return -1;
	}

	VERBOSE("Transferring BL3-0 image to SCP\n");

	/* Transfer BL3-0 image to SCP */
	scp_boot_message_start();

	BOM_CMD_HEADER->id = BOOT_CMD_DATA;
	cmd_data_payload = BOM_CMD_PAYLOAD;
	cmd_data_payload->offset = (uintptr_t) image - MHU_SECURE_BASE;
	cmd_data_payload->block_size = image_size;

	scp_boot_message_send(sizeof(*cmd_data_payload));
	response = scp_boot_message_wait(sizeof(response));
	scp_boot_message_end();

	if (response != 0) {
		ERROR("SCP BOOT_CMD_DATA returned error %u\n", response);
		return -1;
	}

	VERBOSE("Waiting for SCP to signal it is ready to go on\n");

	/* Wait for SCP to signal it's ready */
	return scpi_wait_ready();
}
