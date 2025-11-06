/*
 * Copyright 2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stddef.h>

#include "tbbr_config.h"

#include <firmware_image_package.h>

toc_entry_t plat_def_toc_entries[] = {
	/* DDR PHY firmware */
	{
		.name = "DDR FW",
		.uuid = UUID_DDR_FW,
		.cmdline_name = "ddr-fw"
	},
	{
		.name = NULL,
		.uuid = { {0} },
		.cmdline_name = NULL,
	}
};
