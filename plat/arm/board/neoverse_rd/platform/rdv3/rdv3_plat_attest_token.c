/*
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#include <stdint.h>

#include <cca_attestation.h>
#include <common/debug.h>
#include <plat/common/common_def.h>
#include <psa/error.h>

int plat_rmmd_get_cca_attest_token(uintptr_t buf, size_t *len,
				   uintptr_t hash, size_t hash_size,
				   size_t *remaining_len)
{
	psa_status_t ret;

	assert(*len == SZ_4K);

	ret = cca_attestation_get_plat_token(buf, len, hash, hash_size);
	if (ret != PSA_SUCCESS) {
		ERROR("Unable to fetch CCA attestation token\n");
		return -1;
	}

	assert(*len <= SZ_4K);

	*remaining_len = 0;

	return 0;
}
