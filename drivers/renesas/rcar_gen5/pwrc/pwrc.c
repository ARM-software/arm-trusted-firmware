/*
 * Copyright (c) 2025, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <string.h>

#include <arch.h>
#include <arch_helpers.h>
#include <lib/mmio.h>
#include <plat/common/platform.h>
#include "pwrc.h"
#include "timer.h"

#include "rcar_def.h"
#include "rcar_private.h"

void rcar_pwrc_suspend_to_ram(void)
{
	rcar_pwrc_save_timer_state();

	/* disable MMU */
	disable_mmu_el3();

	/* cache flush */
	dcsw_op_all(DCCISW);

	rcar_scmi_sys_suspend();

	wfi();

	/* Do not return */
	while (true)
		;
}

void rcar_pwrc_code_copy_to_system_ram(void)
{
}

uint32_t rcar_pwrc_cpu_on_check(u_register_t mpidr)
{
	return 0;
}

void rcar_pwrc_enable_interrupt_wakeup(u_register_t mpidr)
{
}

void rcar_pwrc_disable_interrupt_wakeup(u_register_t mpidr)
{
}
