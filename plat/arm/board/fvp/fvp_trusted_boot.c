/*
 * Copyright (c) 2016-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stdint.h>
#include <string.h>

#include <lib/mmio.h>

#include <plat/common/platform.h>
#include <platform_def.h>
#include <tools_share/tbbr_oid.h>

/*
 * Store a new non-volatile counter value.
 *
 * On some FVP versions, the non-volatile counters are read-only so this
 * function will always fail.
 *
 * Return: 0 = success, Otherwise = error
 */
int plat_set_nv_ctr(void *cookie, unsigned int nv_ctr)
{
	const char *oid;
	uintptr_t nv_ctr_addr;

	assert(cookie != NULL);

	oid = (const char *)cookie;
	if (strcmp(oid, TRUSTED_FW_NVCOUNTER_OID) == 0) {
		nv_ctr_addr = TFW_NVCTR_BASE;
	} else if (strcmp(oid, NON_TRUSTED_FW_NVCOUNTER_OID) == 0) {
		nv_ctr_addr = NTFW_CTR_BASE;
	} else {
		return 1;
	}

	mmio_write_32(nv_ctr_addr, nv_ctr);

	/*
	 * If the FVP models a locked counter then its value cannot be updated
	 * and the above write operation has been silently ignored.
	 */
	return (mmio_read_32(nv_ctr_addr) == nv_ctr) ? 0 : 1;
}
