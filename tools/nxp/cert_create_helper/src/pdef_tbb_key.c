/*
 * Copyright 2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <pdef_tbb_key.h>

static key_t pdef_tbb_keys[] = {
	[DDR_FW_CONTENT_KEY - DDR_FW_CONTENT_KEY] = {
		.id = DDR_FW_CONTENT_KEY,
		.opt = "ddr-fw-key",
		.help_msg = "DDR Firmware Content Certificate key (input/output file)",
		.desc = "DDR Firmware Content Certificate key"
	}
};

PLAT_REGISTER_KEYS(pdef_tbb_keys);
