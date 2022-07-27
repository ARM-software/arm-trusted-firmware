/*
 * Copyright (c) 2024, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <drivers/generic_delay_timer.h>
#include <drivers/st/stm32mp_clkfunc.h>
#include <lib/mmio.h>
#include <plat/common/platform.h>
#include <platform_def.h>

#include "stgen_svc.h"
#include <stm32mp2_smc.h>
#include <stm32mp_common.h>
#include <stm32mp_svc_setup.h>

/*
 * This function reads and applies the STGEN frequency value in the STGENC base frequency register,
 * which is the frequency the system base counter use for our platforms.
 */
uint32_t stgen_svc_handler(void)
{
	unsigned long freq_to_set = mmio_read_32(STGEN_BASE + CNTFID_OFF);

	VERBOSE("STGEN frequency set to %lu\n", freq_to_set);

	/*
	 * Update the system counter frequency according to STGEN's base
	 * counter frequency register
	 */
	write_cntfrq_el0((u_register_t)freq_to_set);

	/* Need to update timer with new frequency */
	generic_delay_timer_init();

	return STM32_SMC_OK;
}
