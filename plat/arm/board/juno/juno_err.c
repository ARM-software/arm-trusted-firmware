/*
 * Copyright (c) 2015-2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <errno.h>
#include <v2m_def.h>

#define V2M_SYS_NVFLAGS_ADDR		(V2M_SYSREGS_BASE + V2M_SYS_NVFLAGS)

/*
 * Juno error handler
 */
void plat_error_handler(int err)
{
	uint32_t *flags_ptr = (uint32_t *)V2M_SYS_NVFLAGS_ADDR;

	/* Propagate the err code in the NV-flags register */
	*flags_ptr = err;

	/* Loop until the watchdog resets the system */
	for (;;)
		wfi();
}
