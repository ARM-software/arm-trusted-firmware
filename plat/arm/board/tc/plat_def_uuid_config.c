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
		.name = "RSS Firmware BL1_2 image",
		.uuid = UUID_RSS_FIRMWARE_BL1_2,
		.cmdline_name = "rss-bl1_2"
	},
	{
		.name = "RSS Firmware BL2 image",
		.uuid = UUID_RSS_FIRMWARE_BL2,
		.cmdline_name = "rss-bl2"
	},
	{
		.name = "RSS Firmware SCP BL1 image",
		.uuid = UUID_RSS_FIRMWARE_SCP_BL1,
		.cmdline_name = "rss-scp-bl1"
	},
	{
		.name = "RSS Firmware AP BL1 image",
		.uuid = UUID_RSS_FIRMWARE_AP_BL1,
		.cmdline_name = "rss-ap-bl1"
	},
	{
		.name = "RSS Firmware non-secure image",
		.uuid = UUID_RSS_FIRMWARE_NS,
		.cmdline_name = "rss-ns"
	},
	{
		.name = "RSS Firmware secure image",
		.uuid = UUID_RSS_FIRMWARE_S,
		.cmdline_name = "rss-s"
	},
	{
		.name = "RSS Firmware non-secure SIC tables",
		.uuid = UUID_RSS_SIC_TABLES_NS,
		.cmdline_name = "rss-sic-tables-ns"
	},
	{
		.name = "RSS Firmware secure SIC tables",
		.uuid = UUID_RSS_SIC_TABLES_S,
		.cmdline_name = "rss-sic-tables-s"
	},

	{
		.name = NULL,
		.uuid = { {0} },
		.cmdline_name = NULL,
	}
};
