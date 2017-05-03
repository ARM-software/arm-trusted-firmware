/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <pmu_regs.h>
#include "rk3399_mcu.h"

#define M0_SCR			0xe000ed10  /* System Control Register (SCR) */

#define SCR_SLEEPDEEP_SHIFT	(1 << 2)

void handle_suspend(void)
{
	unsigned int status_value;

	while (1) {
		status_value = mmio_read_32(PMU_BASE + PMU_POWER_ST);
		if (status_value) {
			mmio_clrbits_32(PMU_BASE + PMU_PWRMODE_CON, 0x01);
			return;
		}
	}

	/* m0 enter deep sleep mode */
	mmio_setbits_32(M0_SCR, SCR_SLEEPDEEP_SHIFT);
}
