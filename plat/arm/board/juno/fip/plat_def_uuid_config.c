/*
 * Copyright (c) 2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stddef.h>

#include <firmware_image_package.h>
#include <tbbr_config.h>

#include "plat_def_fip_uuid.h"

toc_entry_t plat_def_toc_entries[] = {
#ifdef ETHOSN_NPU_TZMP1
	ETHOSN_FW_KEY_CERTIFICATE_DEF,
	ETHOSN_FW_CONTENT_CERTIFICATE_DEF,
	ETHOSN_FW_DEF,
#endif /* ETHOSN_NPU_TZMP1 */
	{
		.name = NULL,
		.uuid = { { 0 } },
		.cmdline_name = NULL,
	}
};
