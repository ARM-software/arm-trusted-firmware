/*
 * Copyright (c) 2015-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>

#include <arch_helpers.h>
#include <plat/arm/common/plat_arm.h>
#include <plat/common/platform.h>
#include <platform_def.h>

/*
 * Juno error handler
 */
void __dead2 plat_arm_error_handler(int err)
{
	uint32_t *flags_ptr = (uint32_t *)V2M_SYS_NVFLAGS_ADDR;

	/* Propagate the err code in the NV-flags register */
	*flags_ptr = err;

	/* Loop until the watchdog resets the system */
	for (;;)
		wfi();
}
