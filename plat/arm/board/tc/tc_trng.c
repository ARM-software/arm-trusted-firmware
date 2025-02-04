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
#include <lib/psa/rse_platform_api.h>
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

bool plat_get_entropy(uint64_t *out)
{
#if CRYPTO_SUPPORT
	psa_status_t status;

	status = rse_platform_get_entropy((uint8_t *)out, sizeof(*out));
	if (status != PSA_SUCCESS) {
		printf("Failed for entropy read, psa_status=%d\n", status);
		return false;
	}
#else
	/* Dummy value */
	*out = 0xABBAEDDAACDCDEAD;
#endif

	return true;
}

void plat_entropy_setup(void)
{
	uint64_t entropy;

	plat_trng_uuid = _plat_trng_uuid;

	/* Initialise the entropy source and trigger RNG generation */
	if (!plat_get_entropy(&entropy)) {
		ERROR("Failed to setup entropy\n");
		panic();
	}
}
