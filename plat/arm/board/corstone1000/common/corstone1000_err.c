/*
 * Copyright (c) 2021-2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <plat/arm/common/plat_arm.h>

/*
 * corstone1000 error handler
 */
void __dead2 plat_arm_error_handler(int err)
{
	while (1) {
		wfi();
	}
}
