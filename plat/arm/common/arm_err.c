/*
 * Copyright (c) 2015-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>

#include <plat/arm/common/plat_arm.h>
#include <plat/common/platform.h>

void __dead2 plat_error_handler(int err)
{
	plat_arm_error_handler(err);
}
