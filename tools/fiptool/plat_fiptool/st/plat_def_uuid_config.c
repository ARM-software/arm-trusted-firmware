/*
 * Copyright (c) 2022-2024, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stddef.h>

#include <firmware_image_package.h>

#include "tbbr_config.h"

toc_entry_t plat_def_toc_entries[] = {
	{
		.name = "DDR_FW",
		.uuid = UUID_DDR_FW,
		.cmdline_name = "ddr-fw"
	},
	{
		.name = "STM32MP CONFIG CERT",
		.uuid = UUID_STM32MP_CONFIG_CERT,
		.cmdline_name = "stm32mp-cfg-cert"
	},

	{
		.name = NULL,
		.uuid = { {0} },
		.cmdline_name = NULL,
	}
};
