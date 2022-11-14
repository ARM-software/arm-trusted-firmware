/*
 * Copyright (c) 2015-2022, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "tbbr/tbb_ext.h"
#include "tbbr/tbb_key.h"

#include "tbbr/stm32mp1_tbb_cert.h"

/*
 * Certificates used in the chain of trust
 *
 * The order of the certificates must follow the enumeration specified in
 * stm32mp1_tbb_cert.h. All certificates are self-signed, so the issuer certificate
 * field points to itself.
 */
static cert_t stm32mp1_tbb_certs[] = {
	[0] = {
		.id = STM32MP_CONFIG_CERT,
		.opt = "stm32mp-cfg-cert",
		.help_msg = "STM32MP Config Certificate (output file)",
		.fn = NULL,
		.cn = "STM32MP config FW Certificate",
		.key = ROT_KEY,
		.issuer = STM32MP_CONFIG_CERT,
		.ext = {
			TRUSTED_FW_NVCOUNTER_EXT,
			HW_CONFIG_HASH_EXT,
			FW_CONFIG_HASH_EXT
		},
		.num_ext = 3
	},
};

PLAT_REGISTER_COT(stm32mp1_tbb_certs);
