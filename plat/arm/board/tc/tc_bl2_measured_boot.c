/*
 * Copyright (c) 2022-2024, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include <drivers/arm/rse_comms.h>
#include <drivers/measured_boot/metadata.h>
#include <drivers/measured_boot/rse/rse_measured_boot.h>
#include <tools_share/tbbr_oid.h>

#include <plat/common/common_def.h>
#include <platform_def.h>

/* TC specific table with image IDs and metadata. Intentionally not a
 * const struct, some members might set by bootloaders during trusted boot.
 */
struct rse_mboot_metadata tc_rse_mboot_metadata[] = {
	{
		.id = BL31_IMAGE_ID,
		.slot = U(9),
		.signer_id_size = SIGNER_ID_MIN_SIZE,
		.sw_type = MBOOT_BL31_IMAGE_STRING,
		.pk_oid = BL31_IMAGE_KEY_OID,
		.lock_measurement = true },
	{
		.id = HW_CONFIG_ID,
		.slot = U(10),
		.signer_id_size = SIGNER_ID_MIN_SIZE,
		.sw_type = MBOOT_HW_CONFIG_STRING,
		.pk_oid = HW_CONFIG_KEY_OID,
		.lock_measurement = true },
	{
		.id = SOC_FW_CONFIG_ID,
		.slot = U(11),
		.signer_id_size = SIGNER_ID_MIN_SIZE,
		.sw_type = MBOOT_SOC_FW_CONFIG_STRING,
		.pk_oid = SOC_FW_CONFIG_KEY_OID,
		.lock_measurement = true },
	{
		.id = SCP_BL2_IMAGE_ID,
		.slot = U(12),
		.signer_id_size = SIGNER_ID_MIN_SIZE,
		.sw_type = MBOOT_SCP_BL2_IMAGE_STRING,
		.pk_oid = SCP_BL2_IMAGE_KEY_OID,
		.lock_measurement = true },
	{
		.id = RSE_MBOOT_INVALID_ID }
};

void bl2_plat_mboot_init(void)
{
	/* Initialize the communication channel between AP and RSE */
	(void)rse_comms_init(PLAT_RSE_AP_SND_MHU_BASE,
			     PLAT_RSE_AP_RCV_MHU_BASE);

	rse_measured_boot_init(tc_rse_mboot_metadata);
}

void bl2_plat_mboot_finish(void)
{
	/* Nothing to do. */
}
