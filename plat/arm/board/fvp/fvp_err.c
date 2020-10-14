/*
 * Copyright (c) 2019-2020, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>

#include <common/debug.h>
#include <drivers/arm/sp805.h>
#include <drivers/cfi/v2m_flash.h>
#include <plat/arm/common/plat_arm.h>
#include <platform_def.h>

/*
 * FVP error handler
 */
__dead2 void plat_arm_error_handler(int err)
{
	int ret;

	switch (err) {
	case -ENOENT:
	case -EAUTH:
		/* Image load or authentication error. Erase the ToC */
		INFO("Erasing FIP ToC from flash...\n");
		(void)nor_unlock(PLAT_ARM_FIP_BASE);
		ret = nor_word_program(PLAT_ARM_FIP_BASE, 0);
		if (ret != 0) {
			ERROR("Cannot erase ToC\n");
		} else {
			INFO("Done\n");
		}
		break;
	default:
		/* Unexpected error */
		break;
	}

	console_flush();

	/* Setup the watchdog to reset the system as soon as possible */
	sp805_refresh(ARM_SP805_TWDG_BASE, 1U);

	for (;;)
		wfi();
}
