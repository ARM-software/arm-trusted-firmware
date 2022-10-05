/*
 * Copyright (c) 2022, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <firmware_image_package.h>

#include "tbbr_config.h"

toc_entry_t plat_def_toc_entries[] = {
	{
		.name = "STM32MP CONFIG CERT",
		.uuid = UUID_STM32MP_CONFIG_CERT,
		.cmdline_name = "stm32mp-cfg-cert"
	}
};

