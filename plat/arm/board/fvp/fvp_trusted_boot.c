/*
 * Copyright (c) 2016-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <tbbr_oid.h>

#include "fvp_def.h"

/*
 * Store a new non-volatile counter value. On some FVP versions, the
 * non-volatile counters are RO. On these versions we expect the values in the
 * certificates to always match the RO values so that this function is never
 * called.
 *
 * Return: 0 = success, Otherwise = error
 */
int plat_set_nv_ctr(void *cookie, unsigned int nv_ctr)
{
	const char *oid;
	uint32_t *nv_ctr_addr;

	assert(cookie != NULL);

	oid = (const char *)cookie;
	if (strcmp(oid, TRUSTED_FW_NVCOUNTER_OID) == 0) {
		nv_ctr_addr = (uint32_t *)TFW_NVCTR_BASE;
	} else if (strcmp(oid, NON_TRUSTED_FW_NVCOUNTER_OID) == 0) {
		nv_ctr_addr = (uint32_t *)NTFW_CTR_BASE;
	} else {
		return 1;
	}

	*(unsigned int *)nv_ctr_addr = nv_ctr;

	/* Verify that the current value is the one we just wrote. */
	if (nv_ctr != (unsigned int)(*nv_ctr_addr))
		return 1;

	return 0;
}
