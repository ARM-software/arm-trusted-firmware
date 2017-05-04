/*
 * Copyright (c) 2013-2015, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <generic_delay_timer.h>
#include <mmio.h>
#include <plat_arm.h>
#include <sp804_delay_timer.h>
#include <v2m_def.h>
#include "fvp_def.h"
#include "fvp_private.h"


void bl2_early_platform_setup(meminfo_t *mem_layout)
{
	arm_bl2_early_platform_setup(mem_layout);

	/* Initialize the platform config for future decision making */
	fvp_config_setup();
}

void bl2_platform_setup(void)
{
	arm_bl2_platform_setup();

#if FVP_USE_SP804_TIMER
	/* Enable the clock override for SP804 timer 0, which means that no
	 * clock dividers are applied and the raw (35 MHz) clock will be used */
	mmio_write_32(V2M_SP810_BASE, FVP_SP810_CTRL_TIM0_OV);

	/* Initialize delay timer driver using SP804 dual timer 0 */
	sp804_timer_init(V2M_SP804_TIMER0_BASE,
			SP804_TIMER_CLKMULT, SP804_TIMER_CLKDIV);
#else
	generic_delay_timer_init();
#endif /* FVP_USE_SP804_TIMER */
}
