/*
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

/* Placeholder implementation of the API to retrieve attestation token */
int plat_rmmd_get_cca_attest_token(uintptr_t buf, size_t *len,
				   uintptr_t hash, size_t hash_size)
{
	return 0;
}
