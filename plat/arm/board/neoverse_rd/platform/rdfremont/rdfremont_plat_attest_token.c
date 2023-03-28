/*
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>
#include <stdint.h>

#include <cca_attestation.h>
#include <common/debug.h>
#include <psa/error.h>

int plat_rmmd_get_cca_attest_token(uintptr_t buf, size_t *len,
				   uintptr_t hash, size_t hash_size)
{
	psa_status_t ret;

	ret = cca_attestation_get_plat_token(buf, len, hash, hash_size);
	if (ret != PSA_SUCCESS) {
		ERROR("Unable to fetch CCA attestation token\n");
		return -1;
	}

	return 0;
}
