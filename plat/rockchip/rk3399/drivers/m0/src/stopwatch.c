/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <m0_param.h>
#include "rk3399_mcu.h"

/* use 24MHz SysTick */
#define US_TO_CYCLE(US)	(US * 24)

#define SYST_CST	0xe000e010
/* enable counter */
#define ENABLE		(1 << 0)
/* count down to 0 does not cause SysTick exception to pend */
#define TICKINT		(1 << 1)
/* core clock used for SysTick */
#define CLKSOURCE	(1 << 2)

#define COUNTFLAG	(1 << 16)
#define SYST_RVR	0xe000e014
#define MAX_VALUE	0xffffff
#define MAX_USECS	(MAX_VALUE / US_TO_CYCLE(1))
#define SYST_CVR	0xe000e018
#define SYST_CALIB	0xe000e01c

unsigned int remaining_usecs;

static inline void stopwatch_set_usecs(void)
{
	unsigned int cycle;
	unsigned int usecs = MIN(MAX_USECS, remaining_usecs);

	remaining_usecs -= usecs;
	cycle = US_TO_CYCLE(usecs);
	mmio_write_32(SYST_RVR, cycle);
	mmio_write_32(SYST_CVR, 0);

	mmio_write_32(SYST_CST, ENABLE | TICKINT | CLKSOURCE);
}

void stopwatch_init_usecs_expire(unsigned int usecs)
{
	/*
	 * Enter an inifite loop if the stopwatch is in use. This will allow the
	 * state to be analyzed with a debugger.
	 */
	if (mmio_read_32(SYST_CST) & ENABLE)
		while (1)
			;

	remaining_usecs = usecs;
	stopwatch_set_usecs();
}

int stopwatch_expired(void)
{
	int val = mmio_read_32(SYST_CST);
	if ((val & COUNTFLAG) || !(val & ENABLE)) {
		if (!remaining_usecs)
			return 1;

		stopwatch_set_usecs();
	}

	return 0;
}

void stopwatch_reset(void)
{
	mmio_clrbits_32(SYST_CST, ENABLE);
	remaining_usecs = 0;
}
