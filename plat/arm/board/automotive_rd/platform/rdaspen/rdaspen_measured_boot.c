/*
 * Copyright (c) 2025, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <drivers/arm/mhu.h>
#include <drivers/arm/rse_comms.h>
#include <drivers/measured_boot/metadata.h>
#include <drivers/measured_boot/rse/rse_measured_boot.h>
#include <plat/arm/common/plat_arm.h>
#include <tools_share/tbbr_oid.h>
#include <tools_share/zero_oid.h>

static int plat_rse_comms_init(void);

/*
 * Platform specific table with image IDs and metadata. Intentionally not a
 * const struct, some members might set by bootloaders during trusted boot.
 */
struct rse_mboot_metadata rdaspen_rse_mboot_metadata[] = {
	{
		.id = FW_CONFIG_ID,
		.slot = U(8),
		.signer_id_size = SIGNER_ID_MIN_SIZE,
		.sw_type = MBOOT_FW_CONFIG_STRING,
		.lock_measurement = true,
		.pk_oid = ZERO_OID
	},
	{
		.id = HW_CONFIG_ID,
		.slot = U(9),
		.signer_id_size = SIGNER_ID_MIN_SIZE,
		.sw_type = MBOOT_HW_CONFIG_STRING,
		.lock_measurement = true,
		.pk_oid = HW_CONFIG_KEY_OID
	},
	{
		.id = BL31_IMAGE_ID,
		.slot = U(10),
		.signer_id_size = SIGNER_ID_MIN_SIZE,
		.sw_type = MBOOT_BL31_IMAGE_STRING,
		.lock_measurement = true,
		.pk_oid = BL31_IMAGE_KEY_OID
	},
	{
		.id = BL32_IMAGE_ID,
		.slot = U(11),
		.signer_id_size = SIGNER_ID_MIN_SIZE,
		.sw_type = MBOOT_BL32_IMAGE_STRING,
		.lock_measurement = true,
		.pk_oid = BL32_IMAGE_KEY_OID
	},
	{
		.id = BL33_IMAGE_ID,
		.slot = U(12),
		.signer_id_size = SIGNER_ID_MIN_SIZE,
		.sw_type = MBOOT_BL33_IMAGE_STRING,
		.lock_measurement = true,
		.pk_oid = BL33_IMAGE_KEY_OID
	},
	{
		.id = RSE_MBOOT_INVALID_ID
	}
};

void bl2_plat_mboot_init(void)
{
	/* Initialize the communication channel between AP and RSE */
	(void)plat_rse_comms_init();

	rse_measured_boot_init(rdaspen_rse_mboot_metadata);
}

void bl2_plat_mboot_finish(void)
{
	/* Nothing to do. */
}

int plat_mboot_measure_image(unsigned int image_id, image_info_t *image_data)
{
	int err;

	/* Calculate image hash and record data in RSE */
	err = rse_mboot_measure_and_record(rdaspen_rse_mboot_metadata,
					   image_data->image_base,
					   image_data->image_size,
					   image_id);
	if (err != 0) {
		ERROR("Measure and record failed for image id %u, err (%i)\n",
		      image_id, err);
	}

	return err;
}

int plat_mboot_measure_key(const void *pk_oid, const void *pk_ptr,
			   size_t pk_len)
{
	return rse_mboot_set_signer_id(rdaspen_rse_mboot_metadata, pk_oid, pk_ptr,
				       pk_len);
}

static int plat_rse_comms_init(void)
{
	struct mhu_addr mhu_addresses;

	/* Get sender and receiver frames for AP-RSE communication */
	mhu_addresses.sender_base = AP_RSE_SECURE_MHU_V3_PBX;
	mhu_addresses.receiver_base = AP_RSE_SECURE_MHU_V3_MBX;

	/* Initialize the communication channel between AP and RSE */
	return rse_mbx_init(&mhu_addresses);
}
