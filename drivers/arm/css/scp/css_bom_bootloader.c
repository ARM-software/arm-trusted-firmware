/*
 * Copyright (c) 2014-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stdint.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/arm/css/css_mhu.h>
#include <drivers/arm/css/css_scp.h>
#include <drivers/arm/css/css_scpi.h>
#include <plat/common/platform.h>
#include <platform_def.h>

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
#define BOM_SHARED_MEM		PLAT_CSS_SCP_COM_SHARED_MEM_BASE
#define BOM_CMD_HEADER		((bom_cmd_t *) BOM_SHARED_MEM)
#define BOM_CMD_PAYLOAD		((void *) (BOM_SHARED_MEM + sizeof(bom_cmd_t)))

typedef struct {
	/* Offset from the base address of the Trusted RAM */
	uint32_t offset;
	uint32_t block_size;
} cmd_data_payload_t;

/*
 * All CSS platforms load SCP_BL2/SCP_BL2U just below BL2 (this is where BL31
 * usually resides except when ARM_BL31_IN_DRAM is
 * set). Ensure that SCP_BL2/SCP_BL2U do not overflow into shared RAM and
 * the tb_fw_config.
 */
CASSERT(SCP_BL2_LIMIT <= BL2_BASE, assert_scp_bl2_overwrite_bl2);
CASSERT(SCP_BL2U_LIMIT <= BL2_BASE, assert_scp_bl2u_overwrite_bl2);

CASSERT(SCP_BL2_BASE >= ARM_TB_FW_CONFIG_LIMIT, assert_scp_bl2_overflow);
CASSERT(SCP_BL2U_BASE >= ARM_TB_FW_CONFIG_LIMIT, assert_scp_bl2u_overflow);

static void scp_boot_message_start(void)
{
	mhu_secure_message_start(BOM_MHU_SLOT_ID);
}

static void scp_boot_message_send(size_t payload_size)
{
	/* Ensure that any write to the BOM payload area is seen by SCP before
	 * we write to the MHU register. If these 2 writes were reordered by
	 * the CPU then SCP would read stale payload data */
	dmbst();

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

	/* Ensure that any read to the BOM payload area is done after reading
	 * the MHU register. If these 2 reads were reordered then the CPU would
	 * read invalid payload data */
	dmbld();

	return *(uint32_t *) BOM_SHARED_MEM;
}

static void scp_boot_message_end(void)
{
	mhu_secure_message_end(BOM_MHU_SLOT_ID);
}

int css_scp_boot_image_xfer(void *image, unsigned int image_size)
{
	uint32_t response;
	uint32_t checksum;
	cmd_info_payload_t *cmd_info_payload;
	cmd_data_payload_t *cmd_data_payload;

	assert((uintptr_t) image == SCP_BL2_BASE);

	if ((image_size == 0) || (image_size % 4 != 0)) {
		ERROR("Invalid size for the SCP_BL2 image. Must be a multiple of "
			"4 bytes and not zero (current size = 0x%x)\n",
			image_size);
		return -1;
	}

	/* Extract the checksum from the image */
	checksum = *(uint32_t *) image;
	image = (char *) image + sizeof(checksum);
	image_size -= sizeof(checksum);

	mhu_secure_init();

	VERBOSE("Send info about the SCP_BL2 image to be transferred to SCP\n");

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
#if CSS_DETECT_PRE_1_7_0_SCP
	{
		const uint32_t deprecated_scp_nack_cmd = 0x404;
		uint32_t mhu_status;

		VERBOSE("Detecting SCP version incompatibility\n");

		mhu_status = mhu_secure_message_wait();
		if (mhu_status == deprecated_scp_nack_cmd) {
			ERROR("Detected an incompatible version of the SCP firmware.\n");
			ERROR("Only versions from v1.7.0 onwards are supported.\n");
			ERROR("Please update the SCP firmware.\n");
			return -1;
		}

		VERBOSE("SCP version looks OK\n");
	}
#endif /* CSS_DETECT_PRE_1_7_0_SCP */
	response = scp_boot_message_wait(sizeof(response));
	scp_boot_message_end();

	if (response != 0) {
		ERROR("SCP BOOT_CMD_INFO returned error %u\n", response);
		return -1;
	}

	VERBOSE("Transferring SCP_BL2 image to SCP\n");

	/* Transfer SCP_BL2 image to SCP */
	scp_boot_message_start();

	BOM_CMD_HEADER->id = BOOT_CMD_DATA;
	cmd_data_payload = BOM_CMD_PAYLOAD;
	cmd_data_payload->offset = (uintptr_t) image - ARM_TRUSTED_SRAM_BASE;
	cmd_data_payload->block_size = image_size;

	scp_boot_message_send(sizeof(*cmd_data_payload));
	response = scp_boot_message_wait(sizeof(response));
	scp_boot_message_end();

	if (response != 0) {
		ERROR("SCP BOOT_CMD_DATA returned error %u\n", response);
		return -1;
	}

	return 0;
}

int css_scp_boot_ready(void)
{
	VERBOSE("Waiting for SCP to signal it is ready to go on\n");

	/* Wait for SCP to signal it's ready */
	return scpi_wait_ready();
}
