/*
 * Copyright (c) 2022-2025, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include <drivers/measured_boot/metadata.h>
#include <drivers/measured_boot/rse/rse_measured_boot.h>
#include <tools_share/zero_oid.h>

#include <plat/arm/common/plat_arm.h>
#include <platform_def.h>
#include <tc_rse_comms.h>

/* Table with platform specific image IDs and metadata. Intentionally not a
 * const struct, some members might set by bootloaders during trusted boot.
 */
struct rse_mboot_metadata tc_rse_mboot_metadata[] = {
	{
		.id = FW_CONFIG_ID,
		.slot = U(6),
		.signer_id_size = SIGNER_ID_MIN_SIZE,
		.sw_type = MBOOT_FW_CONFIG_STRING,
		.pk_oid = ZERO_OID,
		.lock_measurement = true },
	{
		.id = TB_FW_CONFIG_ID,
		.slot = U(7),
		.signer_id_size = SIGNER_ID_MIN_SIZE,
		.sw_type = MBOOT_TB_FW_CONFIG_STRING,
		.pk_oid = ZERO_OID,
		.lock_measurement = true },
	{
		.id = BL2_IMAGE_ID,
		.slot = U(8),
		.signer_id_size = SIGNER_ID_MIN_SIZE,
		.sw_type = MBOOT_BL2_IMAGE_STRING,
		.pk_oid = ZERO_OID,
		.lock_measurement = true },

	{
		.id = RSE_MBOOT_INVALID_ID }
};

void bl1_plat_mboot_init(void)
{
	/* Initialize the communication channel between AP and RSE */
	(void)plat_rse_comms_init();

	rse_measured_boot_init(tc_rse_mboot_metadata);
}

void bl1_plat_mboot_finish(void)
{
	/* Nothing to do. */
}
