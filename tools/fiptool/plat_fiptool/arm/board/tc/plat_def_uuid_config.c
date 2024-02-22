/*
 * Copyright (c) 2022, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <stddef.h>

#include <firmware_image_package.h>

#include "tbbr_config.h"

toc_entry_t plat_def_toc_entries[] = {
	{
		.name = "RSE Firmware BL1_2 image",
		.uuid = UUID_RSE_FIRMWARE_BL1_2,
		.cmdline_name = "rse-bl1_2"
	},
	{
		.name = "RSE Firmware BL2 image",
		.uuid = UUID_RSE_FIRMWARE_BL2,
		.cmdline_name = "rse-bl2"
	},
	{
		.name = "RSE Firmware SCP BL1 image",
		.uuid = UUID_RSE_FIRMWARE_SCP_BL1,
		.cmdline_name = "rse-scp-bl1"
	},
	{
		.name = "RSE Firmware AP BL1 image",
		.uuid = UUID_RSE_FIRMWARE_AP_BL1,
		.cmdline_name = "rse-ap-bl1"
	},
	{
		.name = "RSE Firmware non-secure image",
		.uuid = UUID_RSE_FIRMWARE_NS,
		.cmdline_name = "rse-ns"
	},
	{
		.name = "RSE Firmware secure image",
		.uuid = UUID_RSE_FIRMWARE_S,
		.cmdline_name = "rse-s"
	},
	{
		.name = "RSE Firmware non-secure SIC tables",
		.uuid = UUID_RSE_SIC_TABLES_NS,
		.cmdline_name = "rse-sic-tables-ns"
	},
	{
		.name = "RSE Firmware secure SIC tables",
		.uuid = UUID_RSE_SIC_TABLES_S,
		.cmdline_name = "rse-sic-tables-s"
	},

	{
		.name = NULL,
		.uuid = { {0} },
		.cmdline_name = NULL,
	}
};
