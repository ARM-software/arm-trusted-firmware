/*
 * Copyright (c) 2019-2021, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>

#include <common/debug.h>
#include <drivers/arm/sp805.h>
#include <drivers/cfi/v2m_flash.h>
#include <lib/mmio.h>
#include <plat/arm/common/plat_arm.h>
#include <platform_def.h>

/*
 * FVP error handler
 */
__dead2 void plat_arm_error_handler(int err)
{
	/* Propagate the err code in the NV-flags register */
	mmio_write_32(V2M_SYS_NVFLAGS_ADDR, (uint32_t)err);

	console_flush();

	/* Setup the watchdog to reset the system as soon as possible */
	sp805_refresh(ARM_SP805_TWDG_BASE, 1U);

	for (;;)
		wfi();
}
