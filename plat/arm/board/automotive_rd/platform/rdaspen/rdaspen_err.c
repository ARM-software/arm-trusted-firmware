/*
 * Copyright (c) 2025, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <drivers/arm/sbsa.h>
#include <plat/arm/common/plat_arm.h>

/*
 * RD-Aspen error handler
 */
void __dead2 plat_arm_error_handler(int err)
{
	console_flush();

	sbsa_wdog_refresh(SBSA_SECURE_WDOG_BASE);

	while (1) {
		wfi();
	}
}
