/*
 * Copyright (c) 2022-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>
#include <string.h>

#include <plat/common/platform.h>

static const uint8_t sample_delegated_key[] = {
	0x20, 0x11, 0xC7, 0xF0, 0x3C, 0xEE, 0x43, 0x25, 0x17, 0x6E,
	0x52, 0x4F, 0x03, 0x3C, 0x0C, 0xE1, 0xE2, 0x1A, 0x76, 0xE6,
	0xC1, 0xA4, 0xF0, 0xB8, 0x39, 0xAA, 0x1D, 0xF6, 0x1E, 0x0E,
	0x8A, 0x5C, 0x8A, 0x05, 0x74, 0x0F, 0x9B, 0x69, 0xEF, 0xA7,
	0xEB, 0x1A, 0x41, 0x85, 0xBD, 0x11, 0x7F, 0x68
};

/*
 * Get the hardcoded delegated realm attestation key as FVP
 * does not support RSS.
 */
int plat_rmmd_get_cca_realm_attest_key(uintptr_t buf, size_t *len,
				       unsigned int type)
{
	if (*len < sizeof(sample_delegated_key)) {
		return -EINVAL;
	}

	(void)memcpy((void *)buf, (const void *)sample_delegated_key,
		     sizeof(sample_delegated_key));
	*len = sizeof(sample_delegated_key);

	return 0;
}
