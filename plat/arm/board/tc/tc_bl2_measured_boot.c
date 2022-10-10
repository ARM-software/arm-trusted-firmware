/*
 * Copyright (c) 2022, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include <drivers/arm/rss_comms.h>
#include <drivers/measured_boot/rss/rss_measured_boot.h>
#include <lib/psa/measured_boot.h>

#include <plat/common/common_def.h>
#include <platform_def.h>

/* TC specific table with image IDs and metadata. Intentionally not a
 * const struct, some members might set by bootloaders during trusted boot.
 */
struct rss_mboot_metadata tc_rss_mboot_metadata[] = {
	{
		.id = BL31_IMAGE_ID,
		.slot = U(9),
		.signer_id_size = SIGNER_ID_MIN_SIZE,
		.sw_type = RSS_MBOOT_BL31_STRING,
		.lock_measurement = true },
	{
		.id = HW_CONFIG_ID,
		.slot = U(10),
		.signer_id_size = SIGNER_ID_MIN_SIZE,
		.sw_type = RSS_MBOOT_HW_CONFIG_STRING,
		.lock_measurement = true },
	{
		.id = SOC_FW_CONFIG_ID,
		.slot = U(11),
		.signer_id_size = SIGNER_ID_MIN_SIZE,
		.sw_type = RSS_MBOOT_SOC_FW_CONFIG_STRING,
		.lock_measurement = true },
	{
		.id = RSS_MBOOT_INVALID_ID }
};

void bl2_plat_mboot_init(void)
{
	/* Initialize the communication channel between AP and RSS */
	(void)rss_comms_init(PLAT_RSS_AP_SND_MHU_BASE,
			     PLAT_RSS_AP_RCV_MHU_BASE);

	rss_measured_boot_init();
}

void bl2_plat_mboot_finish(void)
{
	/* Nothing to do. */
}
