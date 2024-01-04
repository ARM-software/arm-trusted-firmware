/*
 * Copyright (c) 2017-2024, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arm_acle.h>
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include <lib/mmio.h>
#include <lib/smccc.h>
#include <lib/utils_def.h>
#include <plat/common/platform.h>
#include <platform_def.h>
#include <services/trng_svc.h>
#include <smccc_helpers.h>

DEFINE_SVC_UUID2(_plat_trng_uuid,
	0x23523c58, 0x7448, 0x4083, 0x9d, 0x16,
	0xe3, 0xfa, 0xb9, 0xf1, 0x73, 0xbc
);
uuid_t plat_trng_uuid;

/* Dummy implementation */
bool plat_get_entropy(uint64_t *out)
{
	*out = 0xABBAEDDAACDCDEAD;

	return true;
}

void plat_entropy_setup(void)
{
	uint64_t dummy;

	plat_trng_uuid = _plat_trng_uuid;

	/* Initialise the entropy source and trigger RNG generation */
	plat_get_entropy(&dummy);
}
