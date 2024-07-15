/*
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <plat/arm/common/plat_arm.h>

/*
 * rdv3 error handler
 */
void __dead2 plat_arm_error_handler(int err)
{
	while (1) {
		wfi();
	}
}
