/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "rk3399_mcu.h"

#define PMU_PWRMODE_CON		0x20
#define PMU_POWER_ST		0x78

#define M0_SCR			0xe000ed10  /* System Control Register (SCR) */

#define SCR_SLEEPDEEP_SHIFT	(1 << 2)

static void system_wakeup(void)
{
	unsigned int status_value;
	unsigned int mode_con;

	while (1) {
		status_value = readl(PMU_BASE + PMU_POWER_ST);
		if (status_value) {
			mode_con = readl(PMU_BASE + PMU_PWRMODE_CON);
			writel(mode_con & (~0x01),
			       PMU_BASE + PMU_PWRMODE_CON);
			return;
		}
	}
}

int main(void)
{
	unsigned int reg_src;

	system_wakeup();

	reg_src = readl(M0_SCR);

	/* m0 enter deep sleep mode */
	writel(reg_src | SCR_SLEEPDEEP_SHIFT, M0_SCR);

	for (;;)
		__asm volatile("wfi");

	return 0;
}
