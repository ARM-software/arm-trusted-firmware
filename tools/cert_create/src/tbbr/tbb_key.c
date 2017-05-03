/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "tbbr/tbb_key.h"

/*
 * Keys used to establish the chain of trust
 *
 * The order of the keys must follow the enumeration specified in tbb_key.h
 */
static key_t tbb_keys[] = {
	[ROT_KEY] = {
		.id = ROT_KEY,
		.opt = "rot-key",
		.help_msg = "Root Of Trust key (input/output file)",
		.desc = "Root Of Trust key"
	},
	[TRUSTED_WORLD_KEY] = {
		.id = TRUSTED_WORLD_KEY,
		.opt = "trusted-world-key",
		.help_msg = "Trusted World key (input/output file)",
		.desc = "Trusted World key"
	},
	[NON_TRUSTED_WORLD_KEY] = {
		.id = NON_TRUSTED_WORLD_KEY,
		.opt = "non-trusted-world-key",
		.help_msg = "Non Trusted World key (input/output file)",
		.desc = "Non Trusted World key"
	},
	[SCP_FW_CONTENT_CERT_KEY] = {
		.id = SCP_FW_CONTENT_CERT_KEY,
		.opt = "scp-fw-key",
		.help_msg = "SCP Firmware Content Certificate key (input/output file)",
		.desc = "SCP Firmware Content Certificate key"
	},
	[SOC_FW_CONTENT_CERT_KEY] = {
		.id = SOC_FW_CONTENT_CERT_KEY,
		.opt = "soc-fw-key",
		.help_msg = "SoC Firmware Content Certificate key (input/output file)",
		.desc = "SoC Firmware Content Certificate key"
	},
	[TRUSTED_OS_FW_CONTENT_CERT_KEY] = {
		.id = TRUSTED_OS_FW_CONTENT_CERT_KEY,
		.opt = "tos-fw-key",
		.help_msg = "Trusted OS Firmware Content Certificate key (input/output file)",
		.desc = "Trusted OS Firmware Content Certificate key"
	},
	[NON_TRUSTED_FW_CONTENT_CERT_KEY] = {
		.id = NON_TRUSTED_FW_CONTENT_CERT_KEY,
		.opt = "nt-fw-key",
		.help_msg = "Non Trusted Firmware Content Certificate key (input/output file)",
		.desc = "Non Trusted Firmware Content Certificate key"
	}
};

REGISTER_KEYS(tbb_keys);
