/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <arch_helpers.h>
#include <debug.h>
#include <plat_imx8.h>
#include <sci/sci.h>
#include <stdbool.h>

void __dead2 imx_system_off(void)
{
	sc_pm_set_sys_power_mode(ipc_handle, SC_PM_PW_MODE_OFF);
	wfi();
	ERROR("power off failed.\n");
	panic();
}

void __dead2 imx_system_reset(void)
{
	sc_pm_reset(ipc_handle, SC_PM_RESET_TYPE_BOARD);
	wfi();
	ERROR("system reset failed.\n");
	panic();
}

