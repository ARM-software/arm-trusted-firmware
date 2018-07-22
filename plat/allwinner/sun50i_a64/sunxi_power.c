/*
 * Copyright (c) 2017-2018, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2018, Icenowy Zheng <icenowy@aosc.io>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <debug.h>

int sunxi_pmic_setup(void)
{
	/* STUB */
	NOTICE("BL31: STUB PMIC setup code called\n");

	return 0;
}

void __dead2 sunxi_power_down(void)
{
	ERROR("PSCI: Full shutdown not implemented, halting\n");
	wfi();
	panic();
}
