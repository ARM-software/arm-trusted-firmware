/*
 * Copyright (c) 2021, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <plat/arm/common/plat_arm.h>

/*
 * morello error handler
 */
void __dead2 plat_arm_error_handler(int err)
{
	while (true) {
		wfi();
	}
}
