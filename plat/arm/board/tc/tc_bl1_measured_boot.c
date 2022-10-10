/*
 * Copyright (c) 2022, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include <drivers/arm/rss_comms.h>
#include <drivers/measured_boot/rss/rss_measured_boot.h>
#include <lib/psa/measured_boot.h>

#include <plat/arm/common/plat_arm.h>
#include <platform_def.h>

/* Table with platform specific image IDs and metadata. Intentionally not a
 * const struct, some members might set by bootloaders during trusted boot.
 */
struct rss_mboot_metadata tc_rss_mboot_metadata[] = {
	{
		.id = FW_CONFIG_ID,
		.slot = U(6),
		.signer_id_size = SIGNER_ID_MIN_SIZE,
		.sw_type = RSS_MBOOT_FW_CONFIG_STRING,
		.lock_measurement = true },
	{
		.id = TB_FW_CONFIG_ID,
		.slot = U(7),
		.signer_id_size = SIGNER_ID_MIN_SIZE,
		.sw_type = RSS_MBOOT_TB_FW_CONFIG_STRING,
		.lock_measurement = true },
	{
		.id = BL2_IMAGE_ID,
		.slot = U(8),
		.signer_id_size = SIGNER_ID_MIN_SIZE,
		.sw_type = RSS_MBOOT_BL2_STRING,
		.lock_measurement = true },

	{
		.id = RSS_MBOOT_INVALID_ID }
};

void bl1_plat_mboot_init(void)
{
	/* Initialize the communication channel between AP and RSS */
	(void)rss_comms_init(PLAT_RSS_AP_SND_MHU_BASE,
			     PLAT_RSS_AP_RCV_MHU_BASE);

	rss_measured_boot_init();
}

void bl1_plat_mboot_finish(void)
{
	/* Nothing to do. */
}
