/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <pmu_regs.h>
#include "rk3399_mcu.h"

#define M0_SCR			0xe000ed10  /* System Control Register (SCR) */

#define SCR_SLEEPDEEP_SHIFT	(1 << 2)

__attribute__((noreturn)) void m0_main(void)
{
	unsigned int status_value;

	/*
	 * PMU sometimes doesn't clear power mode bit as it's supposed to due
	 * to a hardware bug. Make the M0 clear it manually to be sure,
	 * otherwise interrupts some cases with concurrent wake interrupts
	 * we stay asleep forever.
	 */
	while (1) {
		status_value = mmio_read_32(PMU_BASE + PMU_POWER_ST);
		if (status_value) {
			mmio_clrbits_32(PMU_BASE + PMU_PWRMODE_CON, 0x01);
			break;
		}
	}

	/*
	 * FSM power secquence is .. -> ST_INPUT_CLAMP(step.17) -> .. ->
	 * ST_WAKEUP_RESET -> ST_EXT_PWRUP-> ST_RELEASE_CLAMP ->
	 * ST_24M_OSC_EN -> .. -> ST_WAKEUP_RESET_CLR(step.26) -> ..,
	 * INPUT_CLAMP and WAKEUP_RESET will hold the SOC not affect by
	 * power or other single glitch, but WAKEUP_RESET need work with 24MHz,
	 * so between RELEASE_CLAMP and 24M_OSC_EN, there have a chance
	 * that glitch will affect SOC, and mess up SOC status, so we
	 * addressmap_shared software clamp between ST_INPUT_CLAMP and
	 * ST_WAKEUP_RESET_CLR to avoid this happen.
	 */
	while (1) {
		status_value = mmio_read_32(PMU_BASE + PMU_POWER_ST);
		if (status_value >= 17)  {
			mmio_setbits_32(PMU_BASE + PMU_SFT_CON, 0x02);
			break;
		}

	}

	while (1) {
		status_value = mmio_read_32(PMU_BASE + PMU_POWER_ST);
		if (status_value >= 26)  {
			mmio_clrbits_32(PMU_BASE + PMU_SFT_CON, 0x02);
			break;
		}
	}

	for (;;)
		__asm__ volatile ("wfi");
}
