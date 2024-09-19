/*
 * Copyright (c) 2024, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>
#include <stdint.h>

#include <lib/smccc.h>
#include <plat/common/plat_trng.h>

#include <mtk_sip_svc.h>

DEFINE_SVC_UUID2(_plat_trng_uuid,
	0xf6b2c8d9, 0x1abb, 0x4d83, 0xb2, 0x3f,
	0x5c, 0x51, 0xb6, 0xef, 0xfc, 0xaf
);
uuid_t plat_trng_uuid;

void plat_entropy_setup(void)
{
	uint64_t placeholder;

	plat_trng_uuid = _plat_trng_uuid;

	/* Initialise the entropy source and trigger RNG generation */
	plat_get_entropy(&placeholder);
}
