/*
 * Copyright (c) 2015-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>

#include <arch_helpers.h>
#include <drivers/arm/sp805.h>
#include <plat/arm/common/plat_arm.h>
#include <platform_def.h>

/*
 * Juno error handler
 */
void __dead2 plat_arm_error_handler(int err)
{
	uint32_t *flags_ptr = (uint32_t *)V2M_SYS_NVFLAGS_ADDR;

	/* Propagate the err code in the NV-flags register */
	*flags_ptr = err;

	/* Setup the watchdog to reset the system as soon as possible */
	sp805_refresh(ARM_SP805_TWDG_BASE, 1U);

	for (;;)
		wfi();
}
