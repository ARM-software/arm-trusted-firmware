/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <generic_delay_timer.h>
#include <mmio.h>
#include <debug.h>
#include <plat_arm.h>
#include <plat_ls.h>
#include "ls1043_def.h"
#include <delay_timer.h>

void bl2_early_platform_setup2(u_register_t arg0, u_register_t arg1,
		u_register_t arg2, u_register_t arg3)
{
	ls_bl2_early_platform_setup((meminfo_t *)arg1);

	/*
	 * Initialize system level generic timer for Layerscape Socs.
	 */
	ls_delay_timer_init();

}

void bl2_platform_setup(void)
{
	NOTICE(FIRMWARE_WELCOME_STR_LS1043_BL2);
}
