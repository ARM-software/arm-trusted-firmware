/*
 * Copyright (c) 2019, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <drivers/arm/sp804_delay_timer.h>
#include <drivers/generic_delay_timer.h>
#include <lib/mmio.h>
#include <plat/arm/common/plat_arm.h>
#include <plat/common/platform.h>
#include <platform_def.h>

#include "fvp_ve_private.h"

void bl2_early_platform_setup2(u_register_t arg0, u_register_t arg1, u_register_t arg2, u_register_t arg3)
{
	arm_bl2_early_platform_setup((uintptr_t)arg0, (meminfo_t *)arg1);

	/* Initialize the platform config for future decision making */
	fvp_ve_config_setup();
}

void bl2_platform_setup(void)
{
	arm_bl2_platform_setup();

#ifdef FVP_VE_USE_SP804_TIMER
	/*
	 * Enable the clock override for SP804 timer 0, which means that no
	 * clock dividers are applied and the raw (35 MHz) clock will be used
	 */
	mmio_write_32(V2M_SP810_BASE, FVP_SP810_CTRL_TIM0_OV);

	/* Initialize delay timer driver using SP804 dual timer 0 */
	sp804_timer_init(V2M_SP804_TIMER0_BASE,
			SP804_TIMER_CLKMULT, SP804_TIMER_CLKDIV);
#else
	generic_delay_timer_init();
#endif /* FVP_VE_USE_SP804_TIMER */
}
